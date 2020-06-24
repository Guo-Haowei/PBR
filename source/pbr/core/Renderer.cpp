#include "base/Error.h"
#include "Renderer.h"
#include "Window.h"
#include "opengl/GLRenderer.h"
#if TARGET_PLATFORM == PLATFORM_WINDOWS
#   include "direct3d/D3d11Renderer.h"
#endif
#if TARGET_PLATFORM == PLATFORM_MACOS
#   include "metal/MtRenderer.h"
#endif
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
#   include "vulkan/VkRenderer.h"
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
        case RenderApi::OPENGL : return new gl::GLRenderer(pWindow);
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        case RenderApi::DIRECT3D11: return new d3d11::D3d11Renderer(pWindow);
#endif
#if TARGET_PLATFORM == PLATFORM_MACOS
        case RenderApi::METAL: return new mt::MtRenderer(pWindow);
#endif
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
        case RenderApi::VULKAN: return new vk::VkRenderer(pWindow);
#endif
        default: assert(0);
    }
    return nullptr;
}

} // namespace pbr
