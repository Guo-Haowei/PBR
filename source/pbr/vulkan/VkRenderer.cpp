#include "VkRenderer.h"
#include "impl/VkRendererImpl.h"

namespace pbr { namespace vk {

VkRenderer::VkRenderer(const Window* pWindow)
    : Renderer(pWindow)
    , impl(std::make_unique<VkRendererImpl>(pWindow))
{
}

void VkRenderer::Initialize()
{
    impl->Initialize();
}

void VkRenderer::DumpGraphicsCardInfo()
{
    impl->DumpGraphicsCardInfo();
}

void VkRenderer::PrepareGpuResources()
{
    impl->PrepareGpuResources();
}

void VkRenderer::Render(const Camera& camera)
{
    impl->Render(camera);
}

void VkRenderer::Resize(const Extent2i& extent)
{
    impl->Resize(extent);
}

void VkRenderer::Finalize()
{
    impl->Finalize();
}

} } // namespace pbr::vk
