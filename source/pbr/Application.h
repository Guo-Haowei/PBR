#pragma once
#include "Window.h"

namespace pbr {

extern Window::CreateInfo g_windowCreateInfo;

class Application
{
public:
    Application();
    void Run();
private:
    void initialize();
    void finalize();
private:
    Window m_window;
};

} // namespace pbr
