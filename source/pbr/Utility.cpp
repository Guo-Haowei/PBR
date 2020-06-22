#include "Utility.h"
#include "Error.h"
#include <fstream>
#include <streambuf>


namespace pbr { namespace utility {

string readAsciiFile(const char* path)
{
    std::ifstream f(path);
    if (!f.good())
        THROW_EXCEPTION("filesystem: Failed to open file '" + string(path) + "'");

    return string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

string readAsciiFile(const string& path)
{
    return readAsciiFile(path.c_str());
}

} } // namespace pbr::utility
