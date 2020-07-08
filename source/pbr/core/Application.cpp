#include "Application.h"
#include "base/Platform.h"
#include "base/Config.h"
#include "base/Error.h"
#include "Globals.h"
#include <glm/gtc/matrix_transform.hpp>
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include <emscripten.h>
#endif
using std::string;

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

void Application::Run(int argc, const char** argv)
{
    configureScene(argc, argv);

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
    mat4 transform = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 10.0f));
    m_camera.SetTransformation(transform);
    m_camera.SetAspect(-1.0f); // force update
    m_camera.SetFov(glm::radians(60.0f));
    m_cameraController.SetCamera(&m_camera);
}

void Application::Mainloop()
{
    m_window->PollEvents();
    m_cameraController.Update(m_window.get());
    handleKeyInput();
    m_renderer->Render(m_camera);
    m_window->SwapBuffers();
    m_window->PostUpdate();
}

void Application::finalize()
{
    m_renderer->Finalize();
    m_window->Finalize();
}

void Application::handleKeyInput()
{
    if (m_window->IsKeyDown(KEY_0)) // pbr
        g_debug = 0;
    else if (m_window->IsKeyDown(KEY_1)) // albedo
        g_debug = 1;
    else if (m_window->IsKeyDown(KEY_2)) // normal
        g_debug = 2;
    else if (m_window->IsKeyDown(KEY_3)) // metallic
        g_debug = 3;
    else if (m_window->IsKeyDown(KEY_4)) // roughness
        g_debug = 4;
    else if (m_window->IsKeyDown(KEY_5)) // ao
        g_debug = 5;
    else if (m_window->IsKeyDown(KEY_6)) // ao
        g_debug = 6;
}

void Application::configureScene(int argc, const char** argv)
{
    string model = "cerberus";
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
    string env = "background";
#else
    string env = "stairs";
#endif

    if (argc > 1)
        model = argv[1];
    if (argc > 2)
        env = argv[2];

    if (model == "cerberus")
    {
        const mat4 scaling = glm::scale(mat4(1.0f), vec3(0.05f));
        const mat4 rotation = glm::rotate(mat4(1.0f), -glm::radians(90.0f), vec3(0, 1, 0)) *
                              glm::rotate(mat4(1.0f), -glm::radians(90.0f), vec3(1, 0, 0));
        const mat4 translation = glm::translate(mat4(1.0f), vec3(2, 0, 0));
        g_transform = translation * rotation * scaling;
    }
    else if (model == "helmet")
    {
        const mat4 scaling = glm::scale(mat4(1.0f), vec3(3.0f));
        const mat4 rotation = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1, 0, 0));
        g_transform = rotation * scaling;
    }
    else
        THROW_EXCEPTION("model [" + model +"] not found");

    g_env_map_path.append(env).append(".hdr");
    g_model_dir.append(model).push_back('/');
}

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
string g_env_map_path = DATA_DIR "preload/";
#else
string g_env_map_path = DATA_DIR "env/";
#endif

string g_model_dir = DATA_DIR "models/";
mat4 g_transform = mat4(1.0f);
int g_debug;

} // namespace pbr
