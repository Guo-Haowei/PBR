#pragma once
#include "base/Definitions.h"
#include "base/Platform.h"
#include <string>

struct GLFWwindow;

namespace pbr {

class Window
{
public:
    enum { BUTTON_LEFT = 0 }; // TODO: check with glfw
    void Initialize(const WindowCreateInfo& info);
    void Finalize();
    bool ShouldClose() const;
    void PollEvents() const;
    void PostUpdate();
    void SwapBuffers() const;
    float GetAspectRatio() const;
    inline RenderApi GetRenderApi() const { return m_renderApi; }
    inline GLFWwindow* GetInternalWindow() const { return m_pWindow; }
    inline const Extent2i& GetWindowExtent() const { return m_windowExtent; }
    inline const Extent2i& GetFrameBufferExtent() const { return m_framebufferExtent; }
    inline void SetWindowExtent(const Extent2i& extent) { m_windowExtent = extent; }
    inline void SetFrameBufferExtent(const Extent2i& extent) { m_framebufferExtent = extent; }
    inline int IsButtonDown(int button) const { return m_buttons[button]; }
    inline const vec2& GetLastFrameCursorPos() const { return m_lastFrameCursorPos; }
    inline const vec2& GetThisFrameCursorPos() const { return m_thisFrameCursorPos; }
    inline double GetScroll() const { return m_scroll; }
private:
    void setWindowSizeFromCreateInfo(const WindowCreateInfo& info);
    void setWindowHintFromCreateInfo(const WindowCreateInfo& info);

    static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mode);
    static void mouseScrollCallback(GLFWwindow* glfwWindow, double x, double y);
    static void mouseCursorCallback(GLFWwindow* glfwWindow, double x, double y);
private:
    RenderApi           m_renderApi         = RenderApi::UNKNOWN;
    GLFWwindow*         m_pWindow           = nullptr;
    string              m_windowTitle;
    Extent2i            m_windowExtent      = { 0, 0 };
    Extent2i            m_framebufferExtent = { 0, 0 };
    array<int, 3>       m_buttons           = { 0, 0, 0 };
    double              m_scroll            = 0;
    vec2                m_lastFrameCursorPos;
    vec2                m_thisFrameCursorPos;
};

} // namespace pbr
