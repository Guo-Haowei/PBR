#pragma once
#include "Definitions.h"
#include "Platform.h"
#include <string>

namespace pbr {

class Window
{
public:
    void Initialize(const WindowCreateInfo& info);
    void Finalize();
    bool ShouldClose() const;
    void PollEvents() const;
    void SwapBuffers() const;
    inline RenderApi GetRenderApi() const { return m_renderApi; }
    GLFWwindow* GetInternalWindow() const { return m_pWindow; }
    const Extent2i& GetWindowExtent() const { return m_windowExtent; }
    const Extent2i& GetFrameBufferExtent() const { return m_framebufferExtent; }
    void SetWindowExtent(const Extent2i& extent) { m_windowExtent = extent; }
    void SetFrameBufferExtent(const Extent2i& extent) { m_framebufferExtent = extent; }
private:
    void setWindowSizeFromCreateInfo(const WindowCreateInfo& info);
    void setWindowHintFromCreateInfo(const WindowCreateInfo& info);

private:
    RenderApi           m_renderApi         = RenderApi::UNKNOWN;
    GLFWwindow*         m_pWindow           = nullptr;
    string              m_windowTitle;
    Extent2i            m_windowExtent      = { 0, 0 };
    Extent2i            m_framebufferExtent = { 0, 0 };
};

} // namespace pbr
