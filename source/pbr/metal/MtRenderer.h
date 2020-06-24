#pragma once
#include "core/Renderer.h"

namespace pbr { namespace mt {

class MtRenderer : public Renderer
{
public:
    MtRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
};

} } // namespace pbr::mt
