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

TexturedMesh LoadModel(const char* path)
{
    string txtpath(path); txtpath.append("model.txt");
    string binpath(path); binpath.append("model.bin");
    ifstream txt(txtpath);
    if (!txt.is_open())
        THROW_EXCEPTION("Failed to open file \"" + txtpath + "\"");

    ifstream bin(binpath, std::ios_base::out | std::ios::binary);
    if (!bin.is_open())
        THROW_EXCEPTION("Failed to open file \"" + binpath + "\"");

    // dummy loader
    TexturedMesh mesh;
    string str;
    int counter = 0;
    while (txt >> str)
    {
        if (str == "size")
        {
            int size; txt >> size;
            if (counter == 0)
            {
                mesh.indices.resize(size / sizeof(uvec3));
                bin.read(reinterpret_cast<char*>(mesh.indices.data()), size);
            }
            else
            {
                mesh.vertices.resize(size / sizeof(TexturedVertex));
                bin.read(reinterpret_cast<char*>(mesh.vertices.data()), size);
            }
            ++counter;
        }
    }

    txt.close();
    bin.close();

    return mesh;
}
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

Image ReadPng(const char* path, int comp)
{
    Image image;
    unsigned char* data = stbi_load(path, &image.width, &image.height, &image.component, 0);
    if (!data)
        THROW_EXCEPTION("filesystem: Failed to open image '" + string(path) + "'");

    image.dataType = DataType::UINT_8T;
    if (comp == 0)
    {
        image.buffer.pData = data;
        image.buffer.sizeInByte = image.width * image.height * image.component;
    }
    else
    {
        if (!(comp == 4 && image.component == 3))
            THROW_EXCEPTION("image: unsupported component");
        image.buffer.sizeInByte = image.width * image.height * comp;
        image.component = comp;
        unsigned char* buffer = (unsigned char*)malloc(image.buffer.sizeInByte);
        for (int i = 0; i < image.width * image.height; ++i)
        {
            buffer[4 * i] = data[3 * i];
            buffer[4 * i + 1] = data[3 * i + 1];
            buffer[4 * i + 2] = data[3 * i + 2];
            buffer[4 * i + 3] = 0;
        }
        free(data);
        image.buffer.pData = buffer;
    }
    return image;
}

Image ReadPng(const string& path, int comp)
{
    return ReadPng(path.c_str(), comp);
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
