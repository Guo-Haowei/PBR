#include "base/Error.h"
#include "Utility.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fstream>
#include <streambuf>
using std::ifstream;
using std::istreambuf_iterator;
using std::ios;

namespace pbr { namespace utility {

string ReadAsciiFile(const char* path)
{
    ifstream f(path);
    if (!f.good())
        THROW_EXCEPTION("filesystem: Failed to open file '" + string(path) + "'");

    return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
}

string ReadAsciiFile(const string& path)
{
    return ReadAsciiFile(path.c_str());
}

vector<char> ReadBinaryFile(const char* path)
{
    ifstream f(path, ios::ate | ios::binary);
    if (!f.good())
        THROW_EXCEPTION("filesystem: Failed to open file '" + string(path) + "'");

    size_t fileSize = static_cast<size_t>(f.tellg());
    vector<char> buffer(fileSize);
    f.seekg(0);
    f.read(buffer.data(), fileSize);
    f.close();
    return buffer;
}

vector<char> ReadBinaryFile(const string& path)
{
    return ReadBinaryFile(path.c_str());
}

Image ReadHDRImage(const string& path)
{
    return ReadHDRImage(path.c_str());
}

Image ReadHDRImage(const char* path)
{
    Image image;
    float* data = stbi_loadf(path, &image.width, &image.height, &image.component, 0);
    if (!data)
        THROW_EXCEPTION("filesystem: Failed to open image '" + string(path) + "'");

    image.buffer.pData = data;
    image.buffer.sizeInByte = sizeof(float) * image.width * image.height * image.component;
    image.dataType = DataType::FLOAT_32T;
    return image;
}

Image ReadBrdfLUT(const char* path, int size)
{
    ifstream bin(path, ios::out | ios::binary);
    if (!bin.is_open())
        THROW_EXCEPTION("filesystem: Failed to open image '" + string(path) + "'");
    const int sizeInByte = sizeof(float) * 2 * size * size;
    float* buffer = reinterpret_cast<float*>(malloc(sizeInByte));
    bin.read(reinterpret_cast<char*>(buffer), sizeInByte);
    Image image;
    image.component = 2;
    image.buffer.pData = buffer;
    image.buffer.sizeInByte = sizeInByte;
    image.dataType = DataType::FLOAT_32T;
    image.width = image.height = size;
    return image;
}

Image ReadBrdfLUT(const string& path, int size)
{
    return ReadBrdfLUT(path.c_str(), size);
}

bool IsNaN(const mat4& m)
{
    const float* p = &m[0].x;
    for (int i = 0; i < 16; ++i)
        if (glm::isnan(p[i]))
            return true;
    return false;
}

} } // namespace pbr::utility
