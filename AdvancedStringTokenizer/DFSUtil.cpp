/**
 * Copyright (c) 2024 Hibiki Serizawa
 *
 * Description: DFSUtil : Utility for processing DFS file
 *
 * Create Date: September 24, 2024
 * Author: Hibiki Serizawa
 */

#include <iomanip>
#include <map>
#include <string>

#include "Vertica.h"
#include "VerticaDFS.h"
#include "DFSUtil.hpp"

using namespace Vertica;
using namespace std;

/**
 * Write configuration parameters into DFS file.
 */
void DFSUtil::writeParameters(ServerInterface &srvInterface, DFSFileWriter &fileWriter, map<string, string> &parameters)
{
    stringstream ss;
    for (const auto& x : parameters) {
        stringstream buf;
        buf << x.first;
        buf << "=";
        buf << x.second;
        string param = buf.str();
        buf.str("");
        buf << hex << setw(4) << setfill('0') << param.size();
        buf << param;
        ss << buf.str();
        buf.clear();
    }

    fileWriter.write(ss.str().c_str(), ss.str().size());
    ss.clear();
}

/**
 * Read configuration parameters from DFS file.
 */
void DFSUtil::readParameters(ServerInterface &srvInterface, DFSFileReader &fileReader, map<string, string> &parameters)
{
    const size_t fileSize = fileReader.size();

    unsigned char* idxBytes = nullptr;
    try {
        idxBytes = new unsigned char[fileSize];
    } catch (bad_alloc &e) {
        vt_report_error(0, "Could not allocate [%zu] bytes", fileSize);
    }

    const size_t readSize = fileReader.read(idxBytes, fileSize);
    if (readSize == 0) {
        delete []idxBytes;
        return;
    }

    string readString(reinterpret_cast<char*> (idxBytes), fileSize);

    while (!readString.empty()) {
        string paramHex = readString.substr(0, 4);
        size_t paramLen = stoi(paramHex, nullptr, 16);
        string line = readString.substr(4, paramLen);
        string parameter = line.substr(0, line.find_first_of("=", 0));
        string value = line.substr(parameter.size() + 1);
        parameters.emplace(parameter, value);
        readString.erase(0, paramLen + 4);
    }

    delete []idxBytes;
}
