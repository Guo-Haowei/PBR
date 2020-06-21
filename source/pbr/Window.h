#pragma once
#include "Definitions.h"
#include <string>

struct GLFWwindow;

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

private:
    void setWindowSizeFromCreateInfo(const WindowCreateInfo& info);
    void setWindowHintFromCreateInfo(const WindowCreateInfo& info);

private:
    RenderApi           m_renderApi     = RenderApi::UNKNOWN;
    GLFWwindow*         m_pWindow       = nullptr;
    Extent2i            m_windowExtent  = { 0, 0 };
    string              m_windowTitle;
};

} // namespace pbr
