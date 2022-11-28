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
#include <unistd.h>
#include <string.h>
#include <errno.h>

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
        int returnValue = 0;

        // Store the output of malloc_info to UDX_EVENTS
        if (argReader.isNull(0) == true) {
            char *buf;
            size_t size;
            FILE *fp = open_memstream(&buf, &size);
            returnValue = malloc_info(0, fp);
            fclose(fp);
            std::map<std::string, std::string> details;
            details["mallocinfo"] = buf;
            srvInterface.logEvent(details);
            free(buf);
        }
        // Store the output of malloc_info to a file specified by the user
        else {
            VString inputValue = argReader.getStringRef(0);
            std::string filename = inputValue.str();
            if (access(filename.c_str(), F_OK) == 0) {
                vt_report_error(0, "File [%s] already exists", filename.c_str());
            } else {
                FILE *fp = fopen(filename.c_str(), "w");
                if (fp == NULL) {
                    vt_report_error(0, "Failed to open %s file [%s]", filename.c_str(), strerror(errno));
                }
                returnValue = malloc_info(0, fp);
                fclose(fp);
            }
        }

        if (returnValue != 0) {
            vt_report_error(0, "Failed to get malloc info");
        }
        resWriter.setInt(returnValue);
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
        argTypes.addVarchar();
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
