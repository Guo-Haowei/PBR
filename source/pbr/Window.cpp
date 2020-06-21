#include "Window.h"
#include "Platform.h"
#include "Error.h"
#include "Application.h"
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

    glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int w, int h)
    {
        Extent2i extent { w, h };
        reinterpret_cast<pbr::Window*>(glfwGetWindowUserPointer(pWindow))->SetWindowExtent({ w, h });
    });
    glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int w, int h)
    {
        Extent2i extent { w, h };
        reinterpret_cast<pbr::Window*>(glfwGetWindowUserPointer(pWindow))->SetFrameBufferExtent({ w, h });
        Application::GetSingleton().GetRenderer()->Resize(extent);
    });

    if (m_renderApi == RenderApi::OPENGL)
        glfwMakeContextCurrent(m_pWindow);

    glfwGetWindowSize(m_pWindow, &m_windowExtent.width, &m_windowExtent.height);
    glfwGetFramebufferSize(m_pWindow, &m_framebufferExtent.width, &m_framebufferExtent.height);

    std::cout << "************* Debug Info *************\n";
    std::cout << "Window size:       " << m_windowExtent << std::endl;
    std::cout << "Framebuffer size:  " << m_framebufferExtent << std::endl;
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
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        case RenderApi::DIRECT3D11:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            m_windowTitle.append(" (Direct3D 11)");
            break;
#endif
        default:
            throw runtime_error("[Error][Window] use unsupported API");
    }
}

} // namespace pbr