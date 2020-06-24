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
    void uploadConstantUniforms();
    void createGeometries();
    void clearGeometries();
private:
    const Window*   m_pWindow;
    GlslProgram     m_pbrProgram;
    GlslProgram     m_envProgram;
    PerDrawData     m_sphere;
    PerDrawData     m_envMap;
    GLTexture       m_hdrTexture;
};

} } // namespace pbr::gl

