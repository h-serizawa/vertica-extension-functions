/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: KafkaRemoveMagicByte : Remove unexpected data in front of JSON data consumed from Apache Kafka
 *
 * Create Date: March 15, 2024
 * Author: Hibiki Serizawa
 */

#include "BuildInfo.h"
#include "Vertica.h"

using namespace Vertica;

constexpr const char *DEBUG_PARAM = "debug";

/**
 * KafkaRemoveMagicByte : Filter class
 */
class KafkaRemoveMagicByte : public UDFilter
{
    vbool debugFlag; // debug flag

public:

    KafkaRemoveMagicByte(vbool && debug_) : debugFlag(std::move(debug_)) {}

    bool useSideChannel() override
    {
        return true;
    }

    StreamState process(ServerInterface &srvInterface, DataBuffer &input, InputState input_state, DataBuffer &output) override
    {
        ereport(ERROR, (errmsg("process should not be called since processWithMetadata is defined."),
                        errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE)));
        return StreamState::DONE;
    }

    StreamState processWithMetadata(ServerInterface &srvInterface, DataBuffer &input, LengthBuffer &inputLengths, InputState inputState,
                                    DataBuffer &output, LengthBuffer &outputLengths) override
    {
        debugLog(srvInterface, "processWithMetadata starts");
        debugLog(srvInterface, " number of messages: %lu", inputLengths.size);
        debugLog(srvInterface, "   initial input size [%lu] offset [%lu]", input.size, input.offset);
        debugLog(srvInterface, "   initial output size [%lu] offset [%lu]", output.size, output.offset);

        if (inputLengths.offset == inputLengths.size) {
            if (input.offset == input.size && inputState == END_OF_FILE) {
                return DONE;
            }
            vt_report_error(0, "Input is not from a StreamSource");
        } else if (inputLengths.size > outputLengths.size) {
            return OUTPUT_NEEDED;
        }

        while (inputLengths.offset < inputLengths.size) {
            size_t record_len = inputLengths.buf[inputLengths.offset];
            debugLog(srvInterface, " message offset: %lu, length: %lu", inputLengths.offset, record_len);

            if (input.offset + record_len > input.size) {
                VIAssert(inputState != END_OF_FILE);
                debugLog(srvInterface, " INPUT_NEEDED returned. input size: %lu, offset: %lu, record length: %lu", input.size, input.offset, record_len);
                return INPUT_NEEDED;
            } else if (output.offset + record_len > output.size) {
                debugLog(srvInterface, " OUTPUT_NEEDED returned. output size: %lu, offset: %lu, record length: %lu", output.size, output.offset, record_len);
                return OUTPUT_NEEDED;
            }

            char *outbuf = output.buf + output.offset;
            const char *p = input.buf + input.offset;
            for (size_t i = 0; i < record_len; ++p, ++i) {
                if (p[0] == '{') {
                    memcpy(outbuf, p, record_len - i);
                    output.offset += record_len - i;
                    outputLengths.buf[outputLengths.offset] = record_len - i;
                    ++outputLengths.offset;
                    break;
                }
            }

            input.offset += record_len;
            ++inputLengths.offset;
        }

        if (inputState != END_OF_FILE) {
            return INPUT_NEEDED;
        }

        VIAssert(input.offset == input.size);
        return DONE;
    }

    /*
     * Write a debug message to the log file.
     */
    void debugLog(ServerInterface &srvInterface, const char *format, ...)
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
 * KafkaRemoveMagicByteFactory : Filter factory class
 */
class KafkaRemoveMagicByteFactory : public FilterFactory
{
    void getParameterType(ServerInterface &srvInterface, SizedColumnTypes &parameterTypes)
    {
        parameterTypes.addBool(DEBUG_PARAM, { false /* visible */, false /* required */, false /* canBeNull */, "Debug flag", false /* isSortedOnThis */ });
    }

    UDFilter* prepare(ServerInterface &srvInterface, PlanContext&) override
    {
        vbool debugFlag = vbool_false;
        ParamReader paramReader = srvInterface.getParamReader();
        if (paramReader.containsParameter(DEBUG_PARAM)) {
            debugFlag = paramReader.getBoolRef(DEBUG_PARAM);
        }
        return vt_createFuncObject<KafkaRemoveMagicByte>(srvInterface.allocator, std::move(debugFlag));
    }
};

// Register KafkaRemoveMagicByte UDFilter.
RegisterFactory(KafkaRemoveMagicByteFactory);
