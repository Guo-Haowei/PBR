#pragma once
#include "Definitions.h"

namespace pbr { namespace utility {
    extern string readAsciiFile(const char* path);
    extern string readAsciiFile(const string& path);
    extern vector<char> readBinaryFile(const char* path);
    extern vector<char> readBinaryFile(const string& path);
} } // namespace pbr::utility
