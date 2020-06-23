#include "base/Error.h"
#include "Window.h"
#include "Application.h"

namespace pbr {

void Window::Initialize(const WindowCreateInfo& info)
{
    m_renderApi = info.renderApi;

    glfwSetErrorCallback([](int error, const char* desc)
    {
        THROW_EXCEPTION(desc);
    });

    glfwInit();
    setWindowSizeFromCreateInfo(info);
    setWindowHintFromCreateInfo(info);

    m_pWindow = glfwCreateWindow(m_windowExtent.width,
                                 m_windowExtent.height,
                                 m_windowTitle.c_str(),
                                 nullptr, nullptr);

    if (m_pWindow == nullptr)
        THROW_EXCEPTION("GLFW: failed to create window");

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

    glfwSetMouseButtonCallback(m_pWindow, Window::mouseButtonCallback);
    glfwSetScrollCallback(m_pWindow, Window::mouseScrollCallback);
    glfwSetCursorPosCallback(m_pWindow, Window::mouseCursorCallback);

    if (m_renderApi == RenderApi::OPENGL)
        glfwMakeContextCurrent(m_pWindow);

    glfwGetWindowSize(m_pWindow, &m_windowExtent.width, &m_windowExtent.height);
    glfwGetFramebufferSize(m_pWindow, &m_framebufferExtent.width, &m_framebufferExtent.height);

    // set cursor position
    double x, y;
    glfwGetCursorPos(m_pWindow, &x, &y);
    m_thisFrameCursorPos = m_lastFrameCursorPos = vec2(x, y);
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

void Window::PollEvents()
{
    m_scroll = 0;
    m_lastFrameCursorPos = m_thisFrameCursorPos;
    glfwPollEvents();
}

void Window::SwapBuffers() const
{
    if (m_renderApi == RenderApi::OPENGL)
        glfwSwapBuffers(m_pWindow);
}

float Window::GetAspectRatio() const
{
    return static_cast<float>(m_framebufferExtent.width) / static_cast<float>(m_framebufferExtent.height);
}

void Window::setWindowSizeFromCreateInfo(const WindowCreateInfo& info)
{
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
    if (info.windowScale > 0.0f)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        m_windowExtent.width = static_cast<int>(info.windowScale * mode->width);
        m_windowExtent.height = static_cast<int>(info.windowScale * mode->height);
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
    glfwWindowHint(GLFW_RESIZABLE, info.resizable);

    switch (info.renderApi)
    {
        case RenderApi::OPENGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PBR_GL_VERSION_MAJOR);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PBR_GL_VERSION_MINOR);
#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
            break;
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        case RenderApi::DIRECT3D11:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
#endif
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
        case RenderApi::VULKAN:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
#endif
        default:
            THROW_EXCEPTION("Use unsupported API: " + RenderApiToString(info.renderApi));
    }

    m_windowTitle.append(" (").append(RenderApiToString(info.renderApi)).append(")");
}

void Window::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mode)
{
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    if (button > window->m_buttons.size())
        return;

    window->m_buttons[button] = action;
}

void Window::mouseScrollCallback(GLFWwindow* glfwWindow, double x, double y)
{
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->m_scroll = y;
}

void Window::mouseCursorCallback(GLFWwindow* glfwWindow, double x, double y)
{
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->m_thisFrameCursorPos = vec2(x, y);
}

} // namespace pbr