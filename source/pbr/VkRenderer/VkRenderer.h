#pragma once
#include "Renderer.h"
#include "vulkan/vulkan.h"

namespace pbr {

class VkRenderer : public Renderer
{
public:
    VkRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void createVkInstance();
private:
    VkInstance m_instance;
};

} // namespace pbr
