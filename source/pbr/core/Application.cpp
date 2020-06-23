#include "Application.h"
#include "base/Platform.h"
#include "base/Config.h"
#include "glm/gtc/matrix_transform.hpp"

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
    cout << "************* Debug Info *************\n";
    m_window.reset(new Window());
    m_window->Initialize(g_windowCreateInfo);
    m_renderer.reset(Renderer::CreateRenderer(m_window.get()));
    m_renderer->Initialize();
    m_renderer->DumpGraphicsCardInfo();
    m_renderer->PrepareGpuResources();

    // initialize camera
    mat4 transform = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 3.0f));
    m_camera.SetTransformation(transform);
    m_camera.SetAspect(-1.0f); // force update
    m_cameraController.SetCamera(&m_camera);
}

void Application::Mainloop()
{
    m_window->PollEvents();
    // update camera
    m_cameraController.Update(m_window.get());
    m_renderer->Render(m_camera);
    m_window->SwapBuffers();
}

void Application::finalize()
{
    m_renderer->Finalize();
    m_window->Finalize();
}

} // namespace pbr
