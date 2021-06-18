#include "D3d11Renderer.h"
#include "impl/D3d11RendererImpl.h"

namespace pbr {
namespace d3d11 {

D3d11Renderer::D3d11Renderer(const Window* pWindow)
    : Renderer(pWindow)
    , impl(std::make_unique<D3d11RendererImpl>(pWindow)) {
}

void D3d11Renderer::Initialize() {
    impl->Initialize();
}

void D3d11Renderer::Finalize() {
    impl->Finalize();
}

void D3d11Renderer::Render(const Camera& camera) {
    impl->Render(camera);
}

void D3d11Renderer::DumpGraphicsCardInfo() {
    impl->DumpGraphicsCardInfo();
}

void D3d11Renderer::PrepareGpuResources() {
    impl->PrepareGpuResources();
}

void D3d11Renderer::Resize(const Extent2i& extent) {
    impl->Resize(extent);
}

}  // namespace d3d11
}  // namespace pbr
