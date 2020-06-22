#pragma once
#include "core/Renderer.h"
#include "GLHelpers.h"

namespace pbr {

class GLRenderer : public Renderer
{
public:
    GLRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void compileShaders();
private:
    GlslProgram m_pbrProgram;
    GLuint m_triangleBuffer = 0;
    GLuint m_triangleVao = 0;
};

} // namespace pbr
