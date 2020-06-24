#pragma once
#include "base/Prerequisites.h"
#include "core/Renderer.h"

namespace pbr { namespace vk {

class VkRendererImpl;

class VkRenderer : public Renderer
{
public:
    VkRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    unique_ptr<VkRendererImpl> impl;
};

} } // namespace pbr::vk
