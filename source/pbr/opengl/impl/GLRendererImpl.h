#pragma once
#include "GLPrerequisites.h"
#include "GLHelpers.h"
#include "core/Camera.h"
#include "core/Window.h"

namespace pbr { namespace gl {

class GLRendererImpl
{
public:
    GLRendererImpl(const Window* pWindow);
    void Initialize();
    void DumpGraphicsCardInfo();
    void PrepareGpuResources();
    void Render(const Camera& camera);
    void Resize(const Extent2i& extent);
    void Finalize();
private:
    void compileShaders();
    void uploadLightUniforms();
    void createSphereBuffers();
    void destroySphereBuffers();
private:
    GlslProgram m_pbrProgram;
    PerDrawData m_sphere;
    const Window* m_pWindow;
};

} } // namespace pbr::gl

