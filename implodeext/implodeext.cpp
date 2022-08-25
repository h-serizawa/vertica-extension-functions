/**
 * Copyright (c) 2022 Hibiki Serizawa
 *
 * Description: ImplodeExt : Extended Implode to support Complex-Type data
 *
 * Create Date: August 23, 2022
 * Author: Hibiki Serizawa
 *   Special thanks to Serge Bonte
 */
#include "Vertica.h"
#include "Arrays/Accessors.h"

using namespace Vertica;

// default maximum number of elements of output array
const int DEFAULT_MAX_ELEMENTS = 256;
// parameter name for maximum number of elements of output array
const std::string MAX_ELEMENTS = "max_elements";
// parameter name for flag to truncate results when output array length exceeds maximum number of elements specified by max_elements parameter
const std::string ALLOW_TRUNCATE = "allow_truncate";
// parameter name for debug flag
const std::string DEBUG = "debug";

//// class ImplodeExt : public CursorTransformFunction
class ImplodeExt : public TransformFunction
{
    ////    ParallelismInfo *pinfo; // store for parallelism situation

    int maxNumOfElements; // maximum number of elements of output array
    vbool truncateFlag;   // flag to truncate results when exceeding max number of elements
    vbool debugFlag;      // debug flag

public:
    void
    setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes)
    {
        ParamReader paramReader = srvInterface.getParamReader();
        if (paramReader.containsParameter(DEBUG)) {
            debugFlag = paramReader.getBoolRef(DEBUG);
            debugLog(srvInterface, "  Debug flag has been enabled");
        } else {
            debugFlag = vbool_false;
        }

        if (paramReader.containsParameter(ALLOW_TRUNCATE)) {
            truncateFlag = paramReader.getBoolRef(ALLOW_TRUNCATE);
            debugLog(srvInterface, "  Allow truncate flag has been enabled");
        } else {
            truncateFlag = vbool_false;
        }

        if (paramReader.containsParameter(MAX_ELEMENTS)) {
            maxNumOfElements = paramReader.getIntRef(MAX_ELEMENTS);
            debugLog(srvInterface, "  Max number of elements has been set to %d", maxNumOfElements);
        } else {
            maxNumOfElements = DEFAULT_MAX_ELEMENTS;
        }
    }

    //    void
    //    setParallelismInfo(ServerInterface &srvInterface,
    //                      ParallelismInfo *parallel)
    //    {
    //        pinfo = parallel;
    //    }

    void
    processPartition(ServerInterface &srvInterface, PartitionReader &inputReader,
                     PartitionWriter &outputWriter) override
    {
        try {
            Array::ArrayWriter aw = outputWriter.getArrayRef(0);
            int elements = 0;

            bool lastNxt = true, anyIters = false;
            while (inputReader.hasMoreData() && !isCanceled()) {
                anyIters = true;
                if (!lastNxt) {
                    vt_report_error(ERRCODE_USER_PROC_EXEC_ERROR,
                                    "Inconsistency between hasMoreData()=true and next()=false");
                }
                if (elements < maxNumOfElements) {
                    aw->copyFromInput(0, inputReader, 0);
                    aw->next();
                    elements++;
                } else if (!truncateFlag) {
                    vt_report_error(ERRCODE_ARRAY_ELEMENT_ERROR,
                                    "Number of elements exceeded max number (%s = %d)",
                                    MAX_ELEMENTS.c_str(), maxNumOfElements);
                }
                lastNxt = inputReader.next();
            }
            if (lastNxt && anyIters) {
                vt_report_error(ERRCODE_USER_PROC_EXEC_ERROR,
                                "Inconsistency between hasMoreData()=false and next()=true");
            }

            aw.commit();
            outputWriter.next();
        } catch (std::exception &e) {
            vt_report_error(ERRCODE_USER_PROC_EXEC_ERROR,
                            "Exception while processing partition: [%s]", e.what());
        }
    }

private:
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

// class ImplodeExtFactory : public CursorTransformFunctionFactory
class ImplodeExtFactory : public TransformFunctionFactory
{

public:
    void
    getPrototype(ServerInterface &srvInterface, ColumnTypes &argTypes,
                 ColumnTypes &returnType) override
    {
        argTypes.addAny();
        returnType.addAny();
    }

    void
    getReturnType(ServerInterface &srvInterface, const SizedColumnTypes &inputTypes,
                  SizedColumnTypes &outputTypes) override
    {
        std::vector<size_t> argCols;
        inputTypes.getArgumentColumns(argCols);
        if (argCols.size() != 1) {
            vt_report_error(ERRCODE_TOO_MANY_ARGUMENTS, "One argument is expected but %s provided",
                            argCols.size() ? std::to_string(argCols.size()).c_str() : "none");
        }

        ParamReader paramReader = srvInterface.getParamReader();
        int maxElements = DEFAULT_MAX_ELEMENTS;
        if (paramReader.containsParameter(MAX_ELEMENTS)) {
            maxElements = paramReader.getIntRef(MAX_ELEMENTS);
            if (maxElements <= 0) {
                vt_report_error(ERRCODE_INVALID_PARAMETER_VALUE, "%s should be a positive number",
                                MAX_ELEMENTS.c_str());
            }
        }
        outputTypes.addArrayType(inputTypes[0], "implode", maxElements);
    }

    void
    getParameterType(ServerInterface &srvInterface, SizedColumnTypes &parameterTypes)
    {
        using Properties = SizedColumnTypes::Properties;
        {
            parameterTypes.addBool(DEBUG, Properties(false /* visible */, false /* required */,
                                                     false /* canBeNull */, "Debug flag",
                                                     false /* isSortedOnThis */));
        }
        {
            parameterTypes.addInt(MAX_ELEMENTS,
                                  Properties(true /* visible */, false /* required */,
                                             false /* canBeNull */, "Max number of elements",
                                             false /* isSortedOnThis */));
        }
        {
            parameterTypes.addBool(
                ALLOW_TRUNCATE,
                Properties(true /* visible */, false /* required */, false /* canBeNull */,
                           "flag to truncate results when output length exceeds max elements",
                           false /* isSortedOnThis */));
        }
    }

    //    void
    //    getConcurrencyModel(ServerInterface &srvInterface,
    //                        ConcurrencyModel &concModel)
    //    {
    //        concModel.nThreads = -1;
    //        concModel.localConc
    //            = ConcurrencyModel::LocalConcurrencyType::LC_CONTEXTUAL;
    //        concModel.globalConc
    //            = ConcurrencyModel::GlobalConcurrencyType::GC_CONTEXTUAL;
    //    }

    //    CursorTransformFunction *
    TransformFunction *
    createTransformFunction(ServerInterface &srvInterface) override
    { //        CursorTransformFunction *tf
        TransformFunction *tf = vt_createFuncObject<ImplodeExt>(srvInterface.allocator);
        //        tf->runProcessPartitionIfEmpty = false;
        return tf;
    }
};

RegisterFactory(ImplodeExtFactory);
