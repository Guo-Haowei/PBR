#include "Window.h"
#include "Platform.h"
#include "Error.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace pbr {

void Window::Initialize(const WindowCreateInfo& info)
{
    m_renderApi = info.renderApi;

    glfwSetErrorCallback([](int error, const char* desc)
    {
        throw runtime_error("[Error][glfw] " + string(desc));
    });

    glfwInit();
    setWindowSizeFromCreateInfo(info);
    setWindowHintFromCreateInfo(info);

    m_pWindow = glfwCreateWindow(m_windowExtent.width,
                                 m_windowExtent.height,
                                 m_windowTitle.c_str(),
                                 nullptr, nullptr);

    if (m_pWindow == nullptr)
        throw runtime_error("[Error][glfw] failed to create glfw window");

    glfwSetWindowUserPointer(m_pWindow, this);
    glfwMakeContextCurrent(m_pWindow);
}

void Window::Finalize()
{
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_pWindow);
}

void Window::PollEvents() const
{
    glfwPollEvents();
}

void Window::SwapBuffers() const
{
    if (m_renderApi == RenderApi::OPENGL)
        glfwSwapBuffers(m_pWindow);
}

void Window::setWindowSizeFromCreateInfo(const WindowCreateInfo& info)
{
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
    if (info.windowScale > 0.0f)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        m_windowExtent.width = info.windowScale * mode->width;
        m_windowExtent.height = info.windowScale * mode->height;
    }
    else
#endif
    {
        m_windowExtent = info.extent;
    }
}

void Window::setWindowHintFromCreateInfo(const WindowCreateInfo& info)
{
    m_windowTitle = "PBR ";
    // common
    glfwWindowHint(GLFW_RESIZABLE, info.resizable);
    switch (info.renderApi)
    {
        case RenderApi::OPENGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PBR_GL_VERSION_MAJOR);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PBR_GL_VERSION_MINOR);
#if PBR_GL_VERSION >= 430 && defined(_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
            m_windowTitle.append(" (OpenGL)");
            break;
        default:
            assert(0);
    }
}

} // namespace pbr