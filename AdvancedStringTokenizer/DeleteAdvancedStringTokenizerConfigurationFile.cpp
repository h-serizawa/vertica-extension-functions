/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: DeleteAdvancedStringTokenizerConfigurationFile : Delete DFS file for AdvancedStringTokenizer
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
 * DeleteAdvancedStringTokenizerConfigurationFile : Scalar function class
 */
class DeleteAdvancedStringTokenizerConfigurationFile : public ScalarFunction
{

private:
    DFSFile file;
    DFSUtil dfsUtil = DFSUtil();

public:

    /**
     * Perform per instance initialization.
     */
    void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        // Delete DFS file
        file = DFSFile(srvInterface, dfsUtil.FILE_PATH);
        if (!file.exists()) {
            vt_report_error(0, "The DFS file [%s] does not exist", dfsUtil.FILE_PATH.c_str());
        } else {
            file.deleteIt(true);
        }
    }

    /**
     * Process a row.
     */
    void processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter) override
    {
        resWriter.setBool(true);
        resWriter.next();
    }
};

/**
 * DeleteAdvancedStringTokenizerConfigurationFileFactory : Scalar function factory class
 */
class DeleteAdvancedStringTokenizerConfigurationFileFactory : public ScalarFunctionFactory
{

public:

    /**
     * Define arguments and outputs.
     */
    void getPrototype(ServerInterface &interface, ColumnTypes &argTypes, ColumnTypes &returnType) override
    {
        returnType.addBool(); // success
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
        return vt_createFuncObject<DeleteAdvancedStringTokenizerConfigurationFile>(interface.allocator);
    }
};

RegisterFactory(DeleteAdvancedStringTokenizerConfigurationFileFactory);
