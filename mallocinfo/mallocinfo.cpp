/**
 * Copyright (c) 2022 Hibiki Serizawa
 *
 * Description: MallocInfo : Log the output of malloc_info() system call
 *
 * Create Date: November 15, 2022
 * Author: Hibiki Serizawa
 */
#include "Vertica.h"
#include <malloc.h>

using namespace Vertica;

/**
 * MallocInfo : Scalar function class
 */
class MallocInfo : public ScalarFunction
{
    void
    setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes)
    {
    }

    void
    processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter)
    {
        char *buf;
        size_t size;
        FILE *fp = open_memstream(&buf, &size);
        malloc_info(0, fp);
        fclose(fp);
        std::map<std::string, std::string> details;
        details["mallocinfo"] = buf;
        srvInterface.logEvent(details);
        free(buf);
        resWriter.setInt(0);
        resWriter.next();
    }
};

/**
 * MallocInfoFactory : Scalar function factory class
 */
class MallocInfoFactory : public ScalarFunctionFactory
{
    void
    getPrototype(ServerInterface &srvInterface, ColumnTypes &argTypes, ColumnTypes &returnType)
    {
        returnType.addInt();
    }

    ScalarFunction *
    createScalarFunction(ServerInterface &srvInterface)
    {
        return vt_createFuncObject<MallocInfo>(srvInterface.allocator);
    }
};

// Register MallocInfo UDx.
RegisterFactory(MallocInfoFactory);
