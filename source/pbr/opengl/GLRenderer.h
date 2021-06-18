#pragma once
#include "base/Prerequisites.h"
#include "core/Renderer.h"

namespace pbr {
namespace gl {

class GLRendererImpl;

class GLRenderer : public Renderer {
   public:
    GLRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;

   private:
    unique_ptr<GLRendererImpl> impl;
};

}  // namespace gl
}  // namespace pbr
