#pragma once
#include "base/Definitions.h"

namespace pbr { namespace utility {
    extern string readAsciiFile(const char* path);
    extern string readAsciiFile(const string& path);
    extern vector<char> readBinaryFile(const char* path);
    extern vector<char> readBinaryFile(const string& path);
    extern bool IsNaN(const mat4& m);
} } // namespace pbr::utility
