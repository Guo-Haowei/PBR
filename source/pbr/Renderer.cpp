#include "Renderer.h"
#include "Error.h"
#include "Window.h"
#include "GLRenderer/GLRenderer.h"
#if TARGET_PLATFORM == PLATFORM_WINDOWS
#   include "D3DRenderer/D3D11Renderer.h"
#endif

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
        case RenderApi::DIRECT3D11: return new D3D11Renderer(pWindow);
#elif TARGET_PLATFORM == PLATFORM_MACOS
#endif
        default: assert(0);
    }
    return nullptr;
}

} // namespace pbr
