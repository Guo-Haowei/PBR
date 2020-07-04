#pragma once
#include "base/Definitions.h"
#include "Mesh.h"

namespace pbr { namespace utility {
    extern string ReadAsciiFile(const char* path);
    extern string ReadAsciiFile(const string& path);
    extern vector<char> ReadBinaryFile(const char* path);
    extern vector<char> ReadBinaryFile(const string& path);
    extern Image ReadPng(const char* path, int comp = 0);
    extern Image ReadPng(const string& path, int comp = 0);
    extern Image ReadHDRImage(const char* path);
    extern Image ReadHDRImage(const string& path);
    extern Image ReadBrdfLUT(const char* path, int size);
    extern Image ReadBrdfLUT(const string& path, int size);
    extern bool IsNaN(const mat4& m);
    extern TexturedMesh LoadModel(const char* path);
} } // namespace pbr::utility
