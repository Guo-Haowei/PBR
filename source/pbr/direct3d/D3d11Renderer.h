#pragma once
#include "base/Prerequisites.h"
#include "core/Renderer.h"

namespace pbr { namespace d3d11 {

class D3d11RendererImpl;

class D3d11Renderer : public Renderer
{
public:
    D3d11Renderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    unique_ptr<D3d11RendererImpl> impl;
};

} } // namespace pbr::d3d11
