#include "Application.h"

namespace pbr {

extern Window::CreateInfo g_windowCreateInfo;

Application::Application()
{
}

void Application::Run()
{
    initialize();

    while (!m_window.ShouldClose())
    {
        m_window.PollEvents();
    }

    finalize();
}

void Application::initialize()
{
    m_window.Initialize(g_windowCreateInfo);
}

void Application::finalize()
{
    m_window.Finalize();
}

} // namespace pbr

