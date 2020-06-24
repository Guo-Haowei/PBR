#include "MtRenderer.h"
#include "impl/MtRendererImpl.h"

namespace pbr { namespace mt {

MtRenderer::MtRenderer(const Window* pWindow)
    : Renderer(pWindow)
    , impl(std::make_unique<MtRendererImpl>(pWindow))
{
}

void MtRenderer::Initialize()
{
    impl->Initialize();
}

void MtRenderer::DumpGraphicsCardInfo()
{
    impl->DumpGraphicsCardInfo();
}

void MtRenderer::PrepareGpuResources()
{
    impl->PrepareGpuResources();
}

void MtRenderer::Render(const Camera& camera)
{
    impl->Render(camera);
}

void MtRenderer::Resize(const Extent2i& extent)
{
    impl->Resize(extent);
}

void MtRenderer::Finalize()
{
    impl->Finalize();
}

} } // namespace pbr::mt
