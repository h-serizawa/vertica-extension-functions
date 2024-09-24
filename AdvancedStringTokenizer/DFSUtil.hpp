/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: DFSUtil : Header file of DFSUtil.cpp
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

#include <map>
#include <string>

#include "Vertica.h"
#include "VerticaDFS.h"

using namespace Vertica;
using namespace std;

const string PARAM_STOPWORDSCASEINSENSITIVE = "stopwordscaseinsensitive";
const string PARAM_MINORSEPARATORS = "minorseparators";
const string PARAM_MAJORSEPARATORS = "majorseparators";
const string PARAM_MINLENGTH = "minlength";
const string PARAM_MAXLENGTH = "maxlength";

class DFSUtil
{

    public:
        DFSUtil() {}
        const std::string FILE_PATH = "advancedStringTokenizer/config";

        void writeParameters(ServerInterface &srvInterface, DFSFileWriter &fileWriter, map<string, string> &parameters);
        void readParameters(ServerInterface &srvInterface, DFSFileReader &fileReader, map<string, string> &parameters);
};
