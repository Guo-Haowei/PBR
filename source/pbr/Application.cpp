#include "Application.h"
#include "Platform.h"
#include "Config.h"

namespace pbr {

static void mainloop()
{
    Application::GetSingleton().Mainloop();
}

Application::Application()
{
}

Application& Application::GetSingleton()
{
    static Application app;
    return app;
}

void Application::Run()
{
    initialize();

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop(pbr::mainloop, 0, true);
#else
    while (!m_window->ShouldClose())
    {
        mainloop();
    }
#endif

    finalize();
}

void Application::initialize()
{
    std::cout << "************* Debug Info *************\n";
    m_window.reset(new Window());
    m_window->Initialize(g_windowCreateInfo);
    m_renderer.reset(Renderer::CreateRenderer(m_window.get()));
    m_renderer->Initialize();
    m_renderer->DumpGraphicsCardInfo();
    m_renderer->PrepareGpuResources();
}

void Application::Mainloop()
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
