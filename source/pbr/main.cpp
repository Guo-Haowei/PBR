#include "Application.h"
#include <iostream>
#include <stdexcept>

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
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
