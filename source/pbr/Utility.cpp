#include "base/Error.h"
#include "Utility.h"
#include <fstream>
#include <streambuf>
using std::ifstream;
using std::istreambuf_iterator;
using std::ios;

namespace pbr { namespace utility {

string readAsciiFile(const char* path)
{
    ifstream f(path);
    if (!f.good())
        THROW_EXCEPTION("filesystem: Failed to open file '" + string(path) + "'");

    return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
}

string readAsciiFile(const string& path)
{
    return readAsciiFile(path.c_str());
}

vector<char> readBinaryFile(const char* path)
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

vector<char> readBinaryFile(const string& path)
{
    return readBinaryFile(path.c_str());
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
