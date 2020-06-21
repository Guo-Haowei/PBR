#pragma once
#include "Renderer.h"

namespace pbr {

class GLRenderer : public Renderer
{
public:
    GLRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
};

} // namespace pbr
