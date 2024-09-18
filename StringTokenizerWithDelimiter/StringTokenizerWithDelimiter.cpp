/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: StringTokenizerWithDelimiter : String Tokenizer that splits value by user-specified delimiter
 *
 * Create Date: September 12, 2024
 * Author: Hibiki Serizawa
 */

#include "Vertica.h"
#include <algorithm>

using namespace Vertica;
using namespace std;

// Maximum output length
static const size_t MAX_TOKEN_LENGTH = 65000;

/**
 * StringTokenizerWithDelimiter : Transform function class
 */
class StringTokenizerWithDelimiter : public TransformFunction
{

private:
    vector<size_t> inputCols; // Data member to store the passed arguments
    char delimiter;           // Delimiter for tokenizer

    /**
     * Check for pass-through inputs.
     */
    void handlePassThroughInputs(ServerInterface &srvInterface,
                                 PartitionReader &inputReader,
                                 PartitionWriter &outputWriter)
    {
        if (inputCols.size() > 2) {
            size_t outputIdx = 1;
            for (size_t inputIdx = 2; inputIdx < inputCols.size(); inputIdx++) {
                outputWriter.copyFromInput(outputIdx, inputReader, inputIdx);
                outputIdx++;
            }
        }
    }

public:

    /**
     * Perform per instance initialization.
     */
    void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        argTypes.getArgumentColumns(inputCols);
        ParamReader sessionParams = srvInterface.getUDSessionParamReader("library");
        string paramValue = sessionParams.containsParameter("delimiter") ? sessionParams.getStringRef("delimiter").str() : ";";
        if (paramValue.length() != 1) {
            vt_report_error(0, "Function only accepts that 'delimiter' session parameter has 1 character, but %zu had", paramValue.length());
        }
        delimiter = paramValue[0];
    }

    /**
     * Process a set of rows.
     */
    void processPartition(ServerInterface &srvInterface,
                          PartitionReader &inputReader,
                          PartitionWriter &outputWriter) override
    {
        if (inputReader.getNumCols() < 2) {
            vt_report_error(0, "Function only accepts 2 or more arguments, but %zu provided", inputReader.getNumCols());
        }

        do {
            const VString &sentence = inputReader.getStringRef(1);

            if (sentence.isNull()) { // If input string is NULL, then output is NULL as well
                VString &word = outputWriter.getStringRef(0);
                word.setNull();

                handlePassThroughInputs(srvInterface, inputReader, outputWriter);
                outputWriter.next();
            } else {
                size_t word_start = 0, word_end = 0;

                const char *s_data = sentence.data();
                size_t s_len = sentence.length();

                while (word_end < s_len) {
                    for ( ; word_end < s_len && s_data[word_end] != delimiter; word_end++) {}

                    VString &word = outputWriter.getStringRef(0);
                    word.copy(&s_data[word_start], min((word_end - word_start), MAX_TOKEN_LENGTH));
                    word_start = ++word_end;

                    handlePassThroughInputs(srvInterface, inputReader, outputWriter);
                    outputWriter.next();
                }
            }
        } while (inputReader.next());
    }
};

/**
 * StringTokenizerWithDelimiterFactory : Transform function factory class
 */
class StringTokenizerWithDelimiterFactory : public TransformFunctionFactory
{

public:

    /**
     * Define arguments and outputs.
     */
    void getPrototype(ServerInterface &srvInterface, ColumnTypes &argTypes, ColumnTypes &returnType) override
    {
        argTypes.addAny();
        returnType.addAny();
    }

    /**
     * Register the data type of outputs.
     */
    void getReturnType(ServerInterface &srvInterface,
                       const SizedColumnTypes &inputTypes,
                       SizedColumnTypes &outputTypes) override
    {
        if (inputTypes.getColumnCount() < 2) {
            vt_report_error(0, "Function only accepts 2 or more arguments, but %zu provided", inputTypes.getColumnCount());
        }

        if (!inputTypes.getColumnType(1).isStringType()) {
            vt_report_error(0, "Second argument to tokenizer must be of varchar type.");
        }

        size_t input_len = inputTypes.getColumnType(1).getStringLength();
        outputTypes.addVarchar(min(input_len, MAX_TOKEN_LENGTH), "token");

        // Handle output rows for added pass-through inputs
        std::vector<size_t> argCols;
        inputTypes.getArgumentColumns(argCols);
        size_t colIdx = 0;
        for (std::vector<size_t>::iterator currCol = argCols.begin() + 2; currCol < argCols.end(); currCol++) {
            std::string inputColName = inputTypes.getColumnName(*currCol);
            std::stringstream cname;
            if (inputColName.empty()) {
                cname << "col" << colIdx;
            } else {
                cname << inputColName;
            }
            colIdx++;

            outputTypes.addArg(inputTypes.getColumnType(*currCol), cname.str());
        }
    }

    TransformFunction *createTransformFunction(ServerInterface &srvInterface) override
    {
        return vt_createFuncObj(srvInterface.allocator, StringTokenizerWithDelimiter);
    }
};

RegisterFactory(StringTokenizerWithDelimiterFactory);
