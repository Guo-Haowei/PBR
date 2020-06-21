#include "Renderer.h"
#include "Error.h"
#include "Window.h"
#include "GLRenderer/GLRenderer.h"

namespace pbr {

Renderer::Renderer(const Window* pWindow)
    : m_pWindow(pWindow)
{
}

Renderer* Renderer::CreateRenderer(const Window* pWindow)
{
    switch (pWindow->GetRenderApi())
    {
        case RenderApi::OPENGL : return new GLRenderer(pWindow);
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        // case RenderApi::DIRECT3D11 : return new GLRenderer(pWindow);
#elif TARGET_PLATFORM == PLATFORM_MACOS
#elif TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#error "Unsupported platform"
#endif
        default: assert(0);
    }
    return nullptr;
}

} // namespace pbr
