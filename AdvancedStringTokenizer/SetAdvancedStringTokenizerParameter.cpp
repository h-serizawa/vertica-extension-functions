/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: SetAdvancedStringTokenizerParameters : Set configuration parameters for AdvancedStringTokenizer and store them into DFS file
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

#include <map>
#include <string>

#include "Vertica.h"
#include "VerticaDFS.h"
#include "DFSUtil.hpp"

using namespace Vertica;
using namespace std;

/**
 * SetAdvancedStringTokenizerParameter : Scalar function class
 */
class SetAdvancedStringTokenizerParameter : public ScalarFunction
{

private:
    DFSFile file;
    DFSFileWriter fileWriter;
    DFSFileReader fileReader;
    DFSUtil dfsUtil = DFSUtil();
    map<string, string> parameters;

public:

    /**
     * Perform per instance initialization.
     */
    void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        // Open DFS file
        file = DFSFile(srvInterface, dfsUtil.FILE_PATH);
        if (!file.exists()) { // If not exist, set default parameters
            parameters.emplace(PARAM_STOPWORDSCASEINSENSITIVE, "");
            parameters.emplace(PARAM_MINORSEPARATORS, "/:=@.-$#%\\_");
            parameters.emplace(PARAM_MAJORSEPARATORS, " []<>(){}|!;,'\"*&?+\r\n\t");
            parameters.emplace(PARAM_MINLENGTH, "2");
            parameters.emplace(PARAM_MAXLENGTH, "128");

            file.create(NS_GLOBAL, HINT_REPLICATE);
        } else { // Read the configuration parameters
            fileReader = DFSFileReader(file);
            fileReader.open();
            dfsUtil.readParameters(srvInterface, fileReader, parameters);
            fileReader.close();
        }

        fileWriter = DFSFileWriter(file);
        fileWriter.open();
    }

    /**
     * Destructor.
     */
    void destroy(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        if (fileWriter.isOpen()) {
            fileWriter.close();
        }
        if (fileReader.isOpen()) {
            fileReader.close();
        }
    }

    /**
     * Process a row.
     */
    void processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter) override
    {
        // Read input data
        const VString parameterName = argReader.getStringRef(0);
        const VString value = argReader.getStringRef(1);

        // Check input data
        if (parameterName.isNull()){
            vt_report_error(0, "Invalid parameter; the parameter must not be null.");
        }
        if (value.isNull()){
            vt_report_error(0, "Invalid value for parameter '%s'; the value must not be null.", parameterName.str().c_str());
        }

        // Set input data
        string parameter = parameterName.str();
        transform(parameter.cbegin(), parameter.cend(), parameter.begin(), [](char c) { return tolower(c); });
        decltype(parameters)::iterator it = parameters.find(parameter);
        if (it == parameters.end()) {
            vt_report_error(0, "Invalid parameter '%s'; the parameter list must be checked.", parameter.c_str());
        } else {
            parameters.erase(it);
            parameters.emplace(parameter, value.str());
        }

        // Write input data into DFS file
        dfsUtil.writeParameters(srvInterface, fileWriter, parameters);
        resWriter.setBool(true);
        resWriter.next();
    }
};

/**
 * SetAdvancedStringTokenizerParameterFactory : Scalar function factory class
 */
class SetAdvancedStringTokenizerParameterFactory : public ScalarFunctionFactory
{

public:

    /**
     * Define arguments and outputs.
     */
    void getPrototype(ServerInterface &interface, ColumnTypes &argTypes, ColumnTypes &returnType) override
    {
        argTypes.addVarchar(); // parameter
        argTypes.addVarchar(); // value
        returnType.addBool();  // success
    }

    /**
     * Register the data type of outputs.
     */
    void getReturnType(ServerInterface &srvInterface,
                       const SizedColumnTypes &argTypes,
                       SizedColumnTypes &returnType) override
    {
        returnType.addBool("success");
    }

    ScalarFunction* createScalarFunction(ServerInterface &interface) override
    {
        return vt_createFuncObject<SetAdvancedStringTokenizerParameter>(interface.allocator);
    }
};

RegisterFactory(SetAdvancedStringTokenizerParameterFactory);
