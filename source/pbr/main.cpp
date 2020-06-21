#include "Application.h"
#include <iostream>
#include <stdexcept>

int main()
{
    using namespace pbr;
    Application app;
    try
    {
        app.Run();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
