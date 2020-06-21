#pragma once
#include "Window.h"
#include "Renderer.h"

namespace pbr {

class Application
{
public:
    Application();
    void Run();
private:
    void initialize();
    void mainloop();
    void finalize();
private:
    unique_ptr<Window>      m_window;
    unique_ptr<Renderer>    m_renderer;
};

} // namespace pbr
