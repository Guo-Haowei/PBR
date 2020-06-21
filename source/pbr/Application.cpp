#include "Application.h"
#include "Config.h"

namespace pbr {

Application::Application()
{
}

void Application::Run()
{
    initialize();

    while (!m_window->ShouldClose())
    {
        mainloop();
    }

    finalize();
}

void Application::initialize()
{
    m_window.reset(new Window());
    m_window->Initialize(g_windowCreateInfo);
    m_renderer.reset(Renderer::CreateRenderer(m_window.get()));
    m_renderer->Initialize();
}

void Application::mainloop()
{
    m_window->PollEvents();
    m_renderer->Render();
    m_window->SwapBuffers();
}

void Application::finalize()
{
    m_renderer->Finalize();
    m_window->Finalize();
}

} // namespace pbr
