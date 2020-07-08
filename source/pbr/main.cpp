#include "base/Error.h"
#include "core/Application.h"
using std::cout;
using std::endl;

// force NV card selection
#ifdef _WIN32
#include <Windows.h>
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main(int argc, const char** argv)
{
    try
    {
        pbr::Application::GetSingleton().Run(argc, argv);
    }
    catch(const pbr::Exception& e)
    {
        cout << e << "\n";
        cout << "\nExitting program with 99..." << endl;
        return 99;
    }

    return 0;
}
