#pragma once
#include "core/Renderer.h"
#include "GLHelpers.h"

namespace pbr { namespace gl {

class GLRenderer : public Renderer
{
public:
    GLRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void compileShaders();
    void createSphereBuffers();
    void destroySphereBuffers();
private:
    GlslProgram m_pbrProgram;
    PerDrawData m_sphere;
};

} } // namespace pbr::gl
