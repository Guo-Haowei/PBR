#include "Application.h"
#include "Error.h"
#include <iostream>

// force NV card selection
#ifdef _WIN32
#include <Windows.h>
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main()
{
    try
    {
        pbr::Application::GetSingleton().Run();
    }
    catch(const pbr::Exception& e)
    {
        std::cerr << e << "\n";
        return 99;
    }

    return 0;
}
