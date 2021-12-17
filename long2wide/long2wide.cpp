/**
 * Copyright (c) 2021 Hibiki Serizawa
 *
 * Description: Long2Wide : Transform the long-form data into the wide-form data
 *
 * Create Date: December 17, 2021
 * Author: Hibiki Serizawa
 */
#include "Vertica.h"
#include "EEUDxShared.h"
#include <cstring>
#include <iostream>
#include <istream>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace Vertica;

const std::string ITEM_LIST = "item_list"; // parameter name for item list
const int ITEM_LIST_MAX_LEN
    = 32000000; // maximum length for item_list parameter
const std::string ITEM_RANGE_MIN
    = "item_range_min"; // parameter name for minimum value of item range
const std::string ITEM_RANGE_MAX
    = "item_range_max"; // parameter name for maximum value of item range
const std::string ZERO_IF_NULL
    = "zero_if_null"; // parameter name for flag to show zero value instead of NULL
const std::string DEBUG = "debug"; // parameter name for debug flag

const std::string ITEM_COLUMN = "item_column"; // argument name for item column
const std::string VALUE_COLUMN
    = "value_column"; // argument name for value column

/**
 * Represent a string value using EE::StringValue to initialize VString
 */
template<size_t maxlen> struct InlineStringValue : public EE::StringValue {
    char data[maxlen];
    InlineStringValue() { setSV(this, nullptr, "", 0); }
};

/**
 * Long2Wide : Transform function class
 */
class Long2Wide : public CursorTransformFunction
{
    std::string itemList;           // item_list parameter value
    std::vector<std::string> items; // array of item list values
    int itemsSize = 0;              // size of array of item list values
    BaseDataOID argTypeOIDItemCol;  // data type of item_column
    BaseDataOID argTypeOIDValueCol; // data type of value_column
    ParallelismInfo *pinfo;         // store for parallelism situation
    vbool zeroIfNullFlag
        = vbool_false; // flag to show zero value instead of NULL
    vbool debugFlag;   // debug flag

public:
    /**
     * Perform per instance initialization.
     */
    void
    setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes)
    {
        // Get debug parameter value and store it to instance variable.
        ParamReader paramReader = srvInterface.getParamReader();
        if (paramReader.containsParameter(DEBUG)) {
            debugFlag = paramReader.getBoolRef(DEBUG);
            debugLog(srvInterface, "  Debug flag has been enabled");
        } else {
            debugFlag = vbool_false;
        }

        // Get item_list parameter value and store it to instance variable.
        if (paramReader.containsParameter(ITEM_LIST)) {
            itemList = paramReader.getStringRef(ITEM_LIST).str();
            debugLog(srvInterface, "  Parameter value of item_list is [%s]",
                     itemList.c_str());
        }
        // Get item_range_max and min parameter values and generate item list.
        else if (paramReader.containsParameter(ITEM_RANGE_MAX)) {
            int maxValue = paramReader.getIntRef(ITEM_RANGE_MAX);
            int minValue = 0;
            if (paramReader.containsParameter(ITEM_RANGE_MIN)) {
                minValue = paramReader.getIntRef(ITEM_RANGE_MIN);
            }
            debugLog(
                srvInterface, "  Parameter value of item_range_max is [%d], item_range_min is [%d]",
                maxValue, minValue);
            itemList = generateItemListFromRange(minValue, maxValue);
            debugLog(srvInterface, "  Generated item_list is [%s]", itemList);
        }
        if (itemsSize == 0) {
            separateListToItems(srvInterface);
        }

        // Get zero_if_null parameter value and store it to instance variable.
        if (paramReader.containsParameter(ZERO_IF_NULL)) {
            zeroIfNullFlag = paramReader.getBoolRef(ZERO_IF_NULL);
            debugLog(srvInterface, "  Parameter value of zero_if_null is [%s]",
                     zeroIfNullFlag);
        }

        // Get data type of 2 arguments and store them to instance variables.
        const VerticaType argTypeItemCol = argTypes.getColumnType(0);
        argTypeOIDItemCol = argTypeItemCol.getTypeOid();
        debugLog(srvInterface, "  Data type of item_column is [%s][%lu]",
                 argTypeItemCol.getPrettyPrintStr().c_str(),
                 argTypeOIDItemCol);
        const VerticaType argTypeValueCol = argTypes.getColumnType(1);
        argTypeOIDValueCol = argTypeValueCol.getTypeOid();
        debugLog(srvInterface, "  Data type of value_column is [%s][%lu]",
                 argTypeValueCol.getPrettyPrintStr().c_str(),
                 argTypeOIDValueCol);

        // Verify data type of first argument (item_column) is the
        // supported type, VARCHAR/CHAR/INTEGER/NUMERIC.
        switch (argTypeOIDItemCol) {
        case CharOID:
        case VarcharOID:
        case Int8OID:
        case NumericOID:
            break;
        default:
            vt_report_error(
                0, "%s supports VARCHAR/CHAR/INTEGER/NUMERIC type but %s provided",
                ITEM_COLUMN, argTypeItemCol.getPrettyPrintStr().c_str());
        }

        // Verify data type of second argument (value_column) is the supported
        // type, VARCHAR/CHAR/INTEGER/FLOAT/NUMERIC.
        switch (argTypeOIDValueCol) {
        case CharOID:
        case VarcharOID:
        case Int8OID:
        case Float8OID:
        case NumericOID:
            break;
        default:
            vt_report_error(
                0, "%s supports VARCHAR/CHAR/INTEGER/FLOAT/NUMERIC type but %s provided",
                VALUE_COLUMN, argTypeValueCol.getPrettyPrintStr().c_str());
        }
    }

    /**
     * Set parallelism / concurrency info.
     */
    void
    setParallelismInfo(ServerInterface &srvInterface,
                       ParallelismInfo *parallel)
    {
        pinfo = parallel;
        debugLog(srvInterface, "  Number of peers is [%d]",
                 parallel->getNumPeers());
    }

    /**
     * Process a set of rows.
     */
    void
    processPartition(ServerInterface &srvInterface,
                     PartitionReader &inputReader,
                     PartitionWriter &outputWriter)
    {
        if (debugFlag == vbool_true) {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            debugLog(srvInterface, "  Thread ID is [%lu]", ss.str().c_str());
        }

        try {
            // Initialize an item map using the value of item_list parameter.
            std::unordered_map<std::string, void *> itemMap = makeItemMap();

            // Read input values and set the tuple to the item map.
            bool lastNxt = true, anyIters = false;
            while (inputReader.hasMoreData() && !isCanceled()) {
                anyIters = true;
                if (!lastNxt) {
                    vt_report_error(0, "Inconsistency between "
                                       "hasMoreData()=true and next()=false");
                }
                // Read item_column value.
                std::string item = getArgumentRefAsString(inputReader, 0);
                debugLog(srvInterface, "  Item value read from input is [%s]",
                         item.c_str());
                // If item_column value is null, nothing is done.
                if (item.length() != 0) {
                    // If item_column value is not listed in item list, nothing is done.
                    auto itr = itemMap.find(item);
                    if (itr != itemMap.end()) {
                        // Set value_column value to item map.
                        setInputToItems(srvInterface, inputReader, 1, itemMap,
                                        item);
                    }
                }
                lastNxt = inputReader.next();
            }
            if (lastNxt && anyIters) {
                vt_report_error(0, "Inconsistency between hasMoreData()=false "
                                   "and next()=true");
            }

            // Generate output from the item map.
            setItemValueToOutput(srvInterface, outputWriter, itemMap);
            outputWriter.next();
            itemMap.clear();
        } catch (std::exception &e) {
            vt_report_error(0, "Exception while processing partition: [%s]",
                            e.what());
        }
    }

private:
    /**
     * Get argument reference and convert it to string.
     */
    const std::string
    getArgumentRefAsString(PartitionReader &inputReader, size_t idx)
    {
        std::string item = "";
        switch (argTypeOIDItemCol) {
        case CharOID:
        case VarcharOID: {
            VString valueString = inputReader.getStringRef(idx);
            if (!valueString.isNull()) {
                item = valueString.str();
            }
        } break;
        case Int8OID: {
            vint valueInt = inputReader.getIntRef(idx);
            if (valueInt != vint_null) {
                item = std::to_string(valueInt);
            }
        } break;
        case NumericOID: {
            VNumeric valueNumeric = inputReader.getNumericRef(idx);
            if (!valueNumeric.isNull()) {
                int convertedValueLength = valueNumeric.getPrecision() + 5;
                if (convertedValueLength < 64)
                    convertedValueLength = 64;
                char convertedValue[convertedValueLength];
                valueNumeric.toString(convertedValue, convertedValueLength);
                item = convertedValue;
            }
        } break;
        default:
            break;
        }
        return item;
    }

    /**
     * Read input value and set the tuple to the item map.
     */
    void
    setInputToItems(ServerInterface &srvInterface,
                    PartitionReader &inputReader, size_t idx,
                    std::unordered_map<std::string, void *> &itemMap,
                    std::string &item)
    {
        switch (argTypeOIDValueCol) {
        case CharOID:
        case VarcharOID: {
            const VString *tempString = inputReader.getStringPtr(idx);
            if (!tempString->isNull()) {
                size_t stringLength = tempString->length() + 1;
                char *valueString = vt_allocArray(srvInterface.allocator, char,
                                                  stringLength);
                std::memset(valueString, '\0', stringLength);
                const char *tempStringChar = tempString->str().c_str();
                for (vsize i = 0; i < tempString->length(); ++i) {
                    valueString[i] = tempStringChar[i];
                }
                itemMap[item] = (void *)valueString;
                debugLog(
                    srvInterface, "    String value set to map[%s] is [%s], value inside map is [%s]",
                    item.c_str(), tempString->str().c_str(),
                    (char *)itemMap[item]);
            }
        } break;
        case Int8OID: {
            vint *valueInt = vt_alloc(srvInterface.allocator, vint);
            std::memcpy(valueInt, inputReader.getIntPtr(idx), sizeof(vint));
            if (*valueInt != vint_null) {
                itemMap[item] = valueInt;
                debugLog(
                    srvInterface, "    Integer value set to map[%s] is [%d], value inside map is [%d]",
                    item.c_str(), *valueInt, *((vint *)itemMap[item]));
            }
        } break;
        case Float8OID: {
            vfloat *valueFloat = vt_alloc(srvInterface.allocator, vfloat);
            std::memcpy(valueFloat, inputReader.getFloatPtr(idx),
                        sizeof(vfloat));
            if (!vfloatIsNull(*valueFloat)) {
                itemMap[item] = (void *)valueFloat;
                debugLog(
                    srvInterface, "    Float value set to map[%s] is [%f], value inside map is [%f]",
                    item.c_str(), *valueFloat, *((vfloat *)itemMap[item]));
            }
        } break;
        case NumericOID: {
            const VNumeric *tempNumeric = inputReader.getNumericPtr(idx);
            if (!tempNumeric->isNull()) {
                VNumeric valueNumeric = *tempNumeric;
                itemMap[item] = (void *)&valueNumeric;
                debugLog(
                    srvInterface, "    Numeric value set to map[%s] is [%lf], value inside map is [%f]",
                    item.c_str(), valueNumeric.toFloat(),
                    (*((VNumeric *)itemMap[item])).toFloat());
            }
        } break;
        default:
            break;
        }
    }

    /**
     * Set values in item map to output.
     */
    void
    setItemValueToOutput(ServerInterface &srvInterface,
                         PartitionWriter &outputWriter,
                         std::unordered_map<std::string, void *> &itemMap)
    {
        switch (argTypeOIDValueCol) {
        case CharOID:
        case VarcharOID: {
            for (int i = 0; i < itemsSize; i++) {
                std::string item = items[i];
                auto itr = itemMap.find(item);
                if (itr != itemMap.end() && itr->second != nullptr) {
                    const char *tempString = (char *)(itr->second);
                    size_t stringLength = std::strlen(tempString) + 1;
                    char *valueString = vt_allocArray(srvInterface.allocator,
                                                      char, stringLength);
                    std::memset(valueString, '\0', stringLength);
                    for (size_t j = 0; j < std::strlen(tempString); ++j) {
                        valueString[j] = tempString[j];
                    }

                    InlineStringValue<65000> buf;
                    VString tempVString(&buf);
                    tempVString.alloc(stringLength);
                    tempVString.copy(valueString, stringLength);
                    outputWriter.getColRefForWrite<VString>(i) = tempVString;
                    debugLog(
                        srvInterface,
                        "  String value set to output[%s(index=%d)] is [%s]",
                        item.c_str(), i, tempVString.str().c_str());
                } else {
                    setNullToOutput(srvInterface, outputWriter, i, item);
                }
            }
        } break;
        case Int8OID: {
            for (int i = 0; i < itemsSize; i++) {
                std::string item = items[i];
                auto itr = itemMap.find(item);
                if (itr != itemMap.end() && itr->second != nullptr) {
                    vint valueInt = *((vint *)(itr->second));
                    outputWriter.setInt(i, valueInt);
                    debugLog(
                        srvInterface,
                        "  Integer value set to output[%s(index=%d)] is [%d]",
                        item.c_str(), i, valueInt);
                } else {
                    setNullToOutput(srvInterface, outputWriter, i, item);
                }
            }
        } break;
        case Float8OID: {
            for (int i = 0; i < itemsSize; i++) {
                std::string item = items[i];
                auto itr = itemMap.find(item);
                if (itr != itemMap.end() && itr->second != nullptr) {
                    vfloat valueFloat = *((vfloat *)(itr->second));
                    outputWriter.setFloat(i, valueFloat);
                    debugLog(
                        srvInterface,
                        "  Float value set to output[%s(index=%d)] is [%f]",
                        item.c_str(), i, valueFloat);
                } else {
                    setNullToOutput(srvInterface, outputWriter, i, item);
                }
            }
        } break;
        case NumericOID: {
            for (int i = 0; i < itemsSize; i++) {
                std::string item = items[i];
                auto itr = itemMap.find(item);
                if (itr != itemMap.end() && itr->second != nullptr) {
                    VNumeric valueNumeric = *((VNumeric *)(itr->second));
                    outputWriter.getColRefForWrite<VNumeric>(i) = valueNumeric;
                    debugLog(
                        srvInterface,
                        "  Numeric value set to output[%s(index=%d)] is [%lf]",
                        item.c_str(), i, valueNumeric.toFloat());
                } else {
                    setNullToOutput(srvInterface, outputWriter, i, item);
                }
            }
        } break;
        default:
            break;
        }
    }

    /**
     * Set Null or Zero value to output.
     */
    void
    setNullToOutput(ServerInterface &srvInterface,
                    PartitionWriter &outputWriter, size_t idx,
                    std::string &itemName)
    {
        if (zeroIfNullFlag == vbool_true) {
            switch (argTypeOIDValueCol) {
            case CharOID:
            case VarcharOID: {
                InlineStringValue<65000> buf;
                VString tempVString(&buf);
                char valueString[] = "0";
                tempVString.alloc(2);
                tempVString.copy(valueString, 2);
                outputWriter.getColRefForWrite<VString>(idx) = tempVString;
                debugLog(
                    srvInterface,
                    "  Zero value set to output[%s(index=%d)] that is [%s]",
                    itemName.c_str(), idx, tempVString.str().c_str());
            } break;
            case Int8OID: {
                outputWriter.setInt(idx, 0);
                debugLog(srvInterface,
                         "  Zero value set to output[%s(index=%d)]",
                         itemName.c_str(), idx);
            } break;
            case Float8OID: {
                outputWriter.setFloat(idx, 0);
                debugLog(srvInterface,
                         "  Zero value set to output[%s(index=%d)]",
                         itemName.c_str(), idx);
            } break;
            case NumericOID: {
                uint64 word[4];
                VerticaType numericType(NumericOID,
                                        VerticaType::makeNumericTypeMod(2, 1));
                VNumeric zeroValue(&word[0], numericType.getNumericPrecision(),
                                   numericType.getNumericScale());
                VNumeric::charToNumeric("0.0", numericType, zeroValue);
                outputWriter.getColRefForWrite<VNumeric>(idx) = zeroValue;
                debugLog(
                    srvInterface,
                    "  Zero value set to output[%s(index=%d)] that is [%lf]",
                    itemName.c_str(), idx, zeroValue.toFloat());
            } break;
            default:
                break;
            }
        } else {
            outputWriter.setNull(idx);
            debugLog(srvInterface,
                     "  Null value is set to output[%s(index=%d)]",
                     itemName.c_str(), idx);
        }
    }

    /**
     * Convert comma-separated item list to array(vector).
     */
    void
    separateListToItems(ServerInterface &srvInterface)
    {
        const char del = ',';
        int del_count = str_chnum(itemList.c_str(), del);

        if (del_count != 0) {
            items.resize(del_count + 1);
            std::size_t first = 0;
            std::size_t last = itemList.find_first_of(del);
            std::size_t itemListSize = itemList.size();
            for (int i = 0; first < itemListSize; i++) {
                std::string subStr(itemList, first, last - first);
                items[i] = subStr;
                first = last + 1;
                last = itemList.find_first_of(del, first);
                if (last == std::string::npos) {
                    last = itemListSize;
                }
            }
            itemsSize = items.size();

            if (debugFlag == vbool_true) {
                debugLog(srvInterface, "  Number of items is [%d]", itemsSize);
                for (int i = 0; i < itemsSize; i++) {
                    debugLog(srvInterface, "    items[%d] is [%s]", i,
                             items[i].c_str());
                }
            }
        }
    }

    /**
     * Count the number of delimiter in a string.
     */
    int
    str_chnum(const char str[], char c)
    {
        int count = 0;
        for (int i = 0; str[i] != '\0'; i++) {
            if (str[i] == c) {
                count++;
            }
        }
        return count;
    }

    /*
     * Generate item map according to item list.
     */
    std::unordered_map<std::string, void *>
    makeItemMap()
    {
        std::unordered_map<std::string, void *> itemMap;
        for (int i = 0; i < itemsSize; ++i) {
            std::string item = items[i];
            if (item.size() != 0) {
                itemMap[item] = nullptr;
            }
        }
        return itemMap;
    }

    /*
     * Generate comma-separated item list using range values.
     */
    std::string
    generateItemListFromRange(int minValue, int maxValue)
    {
        std::vector<int> itemList(maxValue - minValue);
        std::iota(itemList.begin(), itemList.end(), minValue);
        std::ostringstream stream;
        for (size_t i = 0; i < itemList.size(); ++i) {
            if (i) {
                stream << ',';
            }
            stream << itemList[i];
        }
        std::string itemListStr = stream.str();
        return itemListStr;
    }

    /*
     * Write a debug message to the log file.
     */
    void
    debugLog(ServerInterface &srvInterface, const char *format, ...)
    {
        if (debugFlag == vbool_true) {
            va_list arg;
            va_start(arg, format);
            srvInterface.vlog(format, arg);
            va_end(arg);
        }
    }
};

/**
 * Long2WideFactory : Transform function factory class
 */
class Long2WideFactory : public CursorTransformFunctionFactory
{
public:
    /**
     * Define arguments and outputs.
     */
    void
    getPrototype(ServerInterface &srvInterface, ColumnTypes &argTypes,
                 ColumnTypes &returnType)
    {
        // As the prototype, this function supports any argument and output
        // data type. Re-define arguments and outputs in getReturnType method.
        argTypes.addAny();
        returnType.addAny();
    }

    /**
     * Register the data type of outputs according to the arguments.
     */
    void
    getReturnType(ServerInterface &srvInterface,
                  const SizedColumnTypes &inputTypes,
                  SizedColumnTypes &outputTypes)
    {
        // Check number and type of arguments.
        // It expects the first argument is item_column, the second argument is value_column.
        std::vector<size_t> argCols;
        inputTypes.getArgumentColumns(argCols);
        if (argCols.size() != 2) {
            vt_report_error(0, "Two arguments are expected but %s provided",
                            argCols.size()
                                ? std::to_string(argCols.size()).c_str()
                                : "none");
        }
        const VerticaType type = inputTypes.getColumnType(argCols[1]);

        // Register output columns using item list.
        ParamReader paramReader = srvInterface.getParamReader();
        if (paramReader.containsParameter(ITEM_LIST)) {
            std::string itemList = paramReader.getStringRef(ITEM_LIST).str();
            std::vector<std::string> items = getItemList(itemList);
            for (std::string item : items) {
                outputTypes.addArg(type, item);
            }
        } else if (paramReader.containsParameter(ITEM_RANGE_MAX)) {
            int maxValue = paramReader.getIntRef(ITEM_RANGE_MAX);
            int minValue = 0;
            if (paramReader.containsParameter(ITEM_RANGE_MIN)) {
                minValue = paramReader.getIntRef(ITEM_RANGE_MIN);
            }
            if (minValue >= maxValue) {
                vt_report_error(
                    0, "%s parameter value has to be greater than %s parameter value",
                    ITEM_RANGE_MAX, ITEM_RANGE_MIN);
            }
            for (int i = minValue; i < maxValue; ++i) {
                outputTypes.addArg(type, std::to_string(i));
            }
        } else {
            vt_report_error(0,
                            "%s parameter or %s parameter has to be provided",
                            ITEM_LIST, ITEM_RANGE_MAX);
        }
    }

    /**
     * Define the parameters.
     */
    void
    getParameterType(ServerInterface &srvInterface,
                     SizedColumnTypes &parameterTypes)
    {
        using Properties = SizedColumnTypes::Properties;
        // Define item_list parameter.
        {
            parameterTypes.addLongVarchar(
                ITEM_LIST_MAX_LEN, ITEM_LIST,
                Properties(true /* visible */, false /* required */,
                           false /* canBeNull */,
                           "Comma separated value to use to make new columns",
                           false /* isSortedOnThis */));
        }
        // Define item_range_min parameter.
        {
            parameterTypes.addInt(
                ITEM_RANGE_MIN,
                Properties(
                    true /* visible */, false /* required */,
                    false /* canBeNull */, "Minimum value for range of sequence number to make new columns",
                    false /* isSortedOnThis */));
        }
        // Define item_range_max parameter.
        {
            parameterTypes.addInt(
                ITEM_RANGE_MAX,
                Properties(
                    true /* visible */, false /* required */,
                    false /* canBeNull */, "Maximum value for range of sequence number to make new columns",
                    false /* isSortedOnThis */));
        }
        // Define zero_if_null parameter
        {
            parameterTypes.addBool(
                ZERO_IF_NULL,
                Properties(true /* visible */, false /* required */,
                           false /* canBeNull */,
                           "Flag to show zero value instead of NULL",
                           false /* isSortedOnThis */));
        }
        // Define debug parameter
        {
            parameterTypes.addBool(
                DEBUG, Properties(false /* visible */, false /* required */,
                                  false /* canBeNull */, "Debug flag",
                                  false /* isSortedOnThis */));
        }
    }

    /**
     * Define the concurrency.
     */
    void
    getConcurrencyModel(ServerInterface &srvInterface,
                        ConcurrencyModel &concModel)
    {
        concModel.nThreads = -1;
        concModel.localConc
            = ConcurrencyModel::LocalConcurrencyType::LC_CONTEXTUAL;
        concModel.globalConc
            = ConcurrencyModel::GlobalConcurrencyType::GC_CONTEXTUAL;
    }

    CursorTransformFunction *
    createTransformFunction(ServerInterface &srvInterface)
    {
        CursorTransformFunction *tf
            = vt_createFuncObject<Long2Wide>(srvInterface.allocator);
        tf->runProcessPartitionIfEmpty = false;
        return tf;
    }

private:
    std::vector<std::string>
    getItemList(std::string &itemList)
    {
        std::vector<std::string> v;
        std::stringstream itemListStream{ itemList };
        std::string buf;
        while (std::getline(itemListStream, buf, ',')) {
            if (!buf.empty()) {
                v.push_back(buf);
            }
        }
        return v;
    }
};

// Register Long2Wide UDx.
RegisterFactory(Long2WideFactory);
