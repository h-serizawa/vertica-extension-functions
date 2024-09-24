/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: ReadAdvancedStringTokenizerConfigurationFile : Show configuration parameters for AdvancedStringTokenizer
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

#include <string>

#include "Vertica.h"
#include "VerticaDFS.h"
#include "DFSUtil.hpp"

using namespace Vertica;
using namespace std;

/**
 * ReadAdvancedStringTokenizerConfigurationFile : Transform function class
 */
class ReadAdvancedStringTokenizerConfigurationFile : public TransformFunction
{

private:
    DFSFile file;
    DFSFileReader fileReader;
    DFSUtil dfsUtil = DFSUtil();
    map<string, string> parameters; // key-value pairs of parameter and value

public:

    /**
     * Perform per instance initialization.
     */
    void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        file = DFSFile(srvInterface, dfsUtil.FILE_PATH);
        if (!file.exists()) {
            vt_report_error(0, "The DFS file [%s] does not exist", dfsUtil.FILE_PATH.c_str());
        } else {
            fileReader = DFSFileReader(file);
            fileReader.open();
            dfsUtil.readParameters(srvInterface, fileReader, parameters);
            fileReader.close();
        }
    }

    /**
     * Destructor.
     */
    void destroy(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        if (fileReader.isOpen()) {
            fileReader.close();
        }
    }

    /**
     * Process a set of rows.
     */
    void processPartition(ServerInterface &srvInterface,
                          PartitionReader &inputReader,
                          PartitionWriter &outputWriter) override
    {
        // Pass configuration parameters to the writer
        for (const auto& x : parameters) {
            VString &wordParam = outputWriter.getStringRef(0);
            wordParam.copy(x.first);
            VString &wordValue = outputWriter.getStringRef(1);
            wordValue.copy(x.second);
            outputWriter.next();
        }
    }
};

/**
 * ReadAdvancedStringTokenizerConfigurationFileFactory : Transform function factory class
 */
class ReadAdvancedStringTokenizerConfigurationFileFactory : public TransformFunctionFactory
{

public:

    /**
     * Define arguments and outputs.
     */
    void getPrototype(ServerInterface &interface, ColumnTypes &argTypes, ColumnTypes &returnType) override
    {
        returnType.addVarchar(); // parameter
        returnType.addVarchar(); // value
    }

    /**
     * Register the data type of outputs.
     */
    void getReturnType(ServerInterface &srvInterface,
                       const SizedColumnTypes &argTypes,
                       SizedColumnTypes &returnType) override
    {
        returnType.addVarchar(MAX_STRING_LENGTH, "parameter");
        returnType.addVarchar(MAX_STRING_LENGTH, "value");
    }

    TransformFunction* createTransformFunction(ServerInterface &interface) override
    {
        return vt_createFuncObject<ReadAdvancedStringTokenizerConfigurationFile>(interface.allocator);
    }
};

RegisterFactory(ReadAdvancedStringTokenizerConfigurationFileFactory);
