/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: AdvancedStringTokenizer : String Tokenizer with Major / Minor separators
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

#include <istream>
#include <sstream>
#include <map>
#include <unordered_set>

#include "Vertica.h"
#include "VerticaDFS.h"
#include "DFSUtil.hpp"

using namespace Vertica;
using namespace std;

/**
 * AdvancedStringTokenizer : Transform function class
 */
class AdvancedStringTokenizer : public TransformFunction
{

private:
    vector<size_t> inputCols; // Data member to store the passed arguments
    unordered_set<string> stopWordsCaseInsensitive = {}; // List of stop words
    string minorSeparators = "";          // Minor separators
    string majorSeparators = "";          // Major separetors
    size_t minLength = 0;                 // Min length of token
    size_t maxLength = MAX_STRING_LENGTH; // Max length of token
    bool prevCharMinorSep = false;        // Flag to indicate the previous character is minor separator
    bool prevCharMajorSep = false;        // Flag to indicate the previous character is major separator

    DFSFile file;
    DFSFileReader fileReader;
    DFSUtil dfsUtil = DFSUtil();
    map<string, string> parameters; // key-value pairs of parameter and value

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

    /**
     * Check if input character is one of major separator.
     */
    bool isMajorSeparator(char data)
    {
        bool flag = isSeparator(data, majorSeparators);
        if (!flag) {
            setPrevCharMajorSep(false);
        }
        return flag;
    }

    /**
     * Check if input character is one of minor separator.
     */
    bool isMinorSeparator(char data)
    {
        bool flag = isSeparator(data, minorSeparators);
        if (!flag) {
            setPrevCharMinorSep(false);
        }
        return flag;
    }

    /**
     * Check if input character is one of separator passed through argument.
     */
    bool isSeparator(char data, string separators)
    {
        for (size_t i = 0; i < separators.size(); ++i) {
            if (data == separators[i]) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get the flag that indicates the previous character is major separator
     */
    bool isPrevCharMajorSep()
    {
        return prevCharMajorSep;
    }

    /**
     * Get the flag that indicates the previous character is minor separator
     */
    bool isPrevCharMinorSep()
    {
        return prevCharMinorSep;
    }

    /**
     * Set the flag that indicates the previous character is major separator
     */
    void setPrevCharMajorSep(bool flag)
    {
        prevCharMajorSep = flag;
    }

    /**
     * Set the flag that indicates the previous character is minor separator
     */
    void setPrevCharMinorSep(bool flag)
    {
        prevCharMinorSep = flag;
    }

    /**
     * Check if input word is one of stop words.
     */
    bool isStopWord(VString word)
    {
        string str = word.str();
        transform(str.cbegin(), str.cend(), str.begin(), [](char c) { return tolower(c); });
        if (!str.empty() && stopWordsCaseInsensitive.find(str) != stopWordsCaseInsensitive.end()) {
            return true;
        }
        return false;
    }

public:

    /**
     * Perform per instance initialization.
     */
    void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) override
    {
        // Get all passed arguments
        argTypes.getArgumentColumns(inputCols);

        // Open DFS file and read the configuration parameters
        file = DFSFile(srvInterface, dfsUtil.FILE_PATH);
        if (!file.exists()) {
            vt_report_error(0, "The DFS file [%s] does not exist", dfsUtil.FILE_PATH.c_str());
        } else {
            fileReader = DFSFileReader(file);
            fileReader.open();
            dfsUtil.readParameters(srvInterface, fileReader, parameters);
            fileReader.close();
        }

        // Set the configuration parameters
        for (const auto& x : parameters) {
            if (x.first == PARAM_STOPWORDSCASEINSENSITIVE) { // stopwordscaseinsensitive
                stringstream ss{x.second};
                string buf;
                while (getline(ss, buf, ',')) {
                    if (!buf.empty()) {
                        stopWordsCaseInsensitive.insert(buf);
                    }
                }
                ss.clear();
            } else if (x.first == PARAM_MINORSEPARATORS) { // minorseparators
                minorSeparators = x.second;
            } else if (x.first == PARAM_MAJORSEPARATORS) { // majorseparators
                majorSeparators = x.second;
            } else if (x.first == PARAM_MINLENGTH) { // minlength
                minLength = stoul(x.second, nullptr, 10);
            } else if (x.first == PARAM_MAXLENGTH) { // maxlength
                maxLength = stoul(x.second, nullptr, 10);
            }
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
        if (inputReader.getNumCols() < 2) {
            vt_report_error(0, "Function only accepts 2 or more arguments, but %zu provided", inputReader.getNumCols());
        }

        // Loop until the reader has the inputs
        do {
            // Read the text field
            const VString &sentence = inputReader.getStringRef(1);

            if (!sentence.isNull()) {
                size_t wordStart = 0, wordEnd = 0, wordMinorStart = 0;
                bool majorFlag = false, minorFlag = false;

                const char *sentenceData = sentence.data();
                size_t sentenseLength = sentence.length();

                while (wordEnd < sentenseLength) {
                    // Skip reading the characters until major/minor separator appears
                    while (wordEnd < sentenseLength) {
                        if (isMajorSeparator(sentenceData[wordEnd])) {
                            majorFlag = true;
                            break;
                        } else if (isMinorSeparator(sentenceData[wordEnd])) {
                            minorFlag = true;
                            break;
                        }
                        ++wordEnd;
                    }

                    // Create a token using minor separator
                    if (minorFlag || wordStart != wordMinorStart) {
                        if (!isPrevCharMajorSep() && !isPrevCharMinorSep() && minLength <= wordEnd - wordMinorStart) {
                            VString &word = outputWriter.getStringRef(0);
                            word.copy(&sentenceData[wordMinorStart], (maxLength < wordEnd - wordMinorStart) ? maxLength : wordEnd - wordMinorStart);
                            if (!isStopWord(word)) {
                                handlePassThroughInputs(srvInterface, inputReader, outputWriter);
                                outputWriter.next();
                            }
                        }

                        wordMinorStart = wordEnd + 1;
                        setPrevCharMinorSep(true);
                        minorFlag = false;
                    }

                    // Create a token using major separator
                    if (majorFlag || sentenseLength == wordEnd || sentenseLength == wordMinorStart) {
                        if (!majorFlag && sentenseLength == wordMinorStart) {
                            ++wordEnd;
                        }
                        if (!isPrevCharMajorSep() && minLength <= wordEnd - wordStart) {
                            VString &word = outputWriter.getStringRef(0);
                            word.copy(&sentenceData[wordStart], (maxLength < wordEnd - wordStart) ? maxLength : wordEnd - wordStart);
                            if (!isStopWord(word)) {
                                handlePassThroughInputs(srvInterface, inputReader, outputWriter);
                                outputWriter.next();
                            }
                        }

                        wordStart = wordEnd + 1;
                        wordMinorStart = wordStart;
                        setPrevCharMajorSep(true);
                        majorFlag = false;
                    }

                    ++wordEnd;
                }
            }
        } while (inputReader.next());
    }

};

/**
 * AdvancedStringTokenizerFactory : Transform function factory class
 */
class AdvancedStringTokenizerFactory : public TransformFunctionFactory
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

        // Set max length to the return columns
        VerticaType inputType = inputTypes.getColumnType(1);
        BaseDataOID inputTypeOID = inputType.getTypeOid();
        size_t inputLen = inputType.getStringLength();
        if (inputTypeOID == VarcharOID) {
            outputTypes.addVarchar(inputLen == 0 ? MAX_STRING_LENGTH : inputLen, "token");
        } else if (inputTypeOID == LongVarcharOID) {
            outputTypes.addLongVarchar(inputLen == 0 ? MAX_LONG_STRING_LENGTH : inputLen, "token");
        } else {
            vt_report_error(0, "Second argument to tokenizer must be of varchar type.");
        }

        // Handle output rows for added pass-through inputs
        vector<size_t> argCols;
        inputTypes.getArgumentColumns(argCols);
        size_t colIdx = 0;
        for (vector<size_t>::iterator currCol = argCols.begin() + 2; currCol < argCols.end(); currCol++) {
            string inputColName = inputTypes.getColumnName(*currCol);
            stringstream cname;
            if (inputColName.empty()) {
                cname << "col" << colIdx;
            } else {
                cname << inputColName;
            }
            colIdx++;

            outputTypes.addArg(inputTypes.getColumnType(*currCol), cname.str());
            cname.clear();
        }
    }

    TransformFunction *createTransformFunction(ServerInterface &srvInterface) override
    {
        return vt_createFuncObj(srvInterface.allocator, AdvancedStringTokenizer);
    }
};

RegisterFactory(AdvancedStringTokenizerFactory);
