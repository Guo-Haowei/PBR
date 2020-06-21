#pragma once
#include "Definitions.h"
#include <string>

struct GLFWwindow;

namespace pbr {

class Window
{
public:
    struct CreateInfo
    {
        Extent2i        extent;
        float           windowScale;
        bool            resizable;
        Renderer        renderer;
        // msaa
        // vsync

        CreateInfo(Renderer renderer, float windowScale = 0.0f, const Extent2i& extent = Extent2i(), bool resizable = true)
            : extent(extent), windowScale(windowScale), resizable(resizable), renderer(renderer) { }
    };

    void Initialize(const CreateInfo& info);
    void Finalize();
    bool ShouldClose() const;
    void PollEvents() const;

private:
    void setWindowSizeFromCreateInfo(const CreateInfo& info);
    void setWindowHintFromCreateInfo(const CreateInfo& info);

private:
    Renderer            m_renderer      = Renderer::UNKNOWN;
    GLFWwindow*         m_pWindow       = nullptr;
    Extent2i            m_windowExtent  = { 0, 0 };
    string              m_windowTitle;
};

} // namespace pbr
