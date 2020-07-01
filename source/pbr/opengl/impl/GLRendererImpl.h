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
    void createFramebuffer();
    void compileShaders();
    void uploadConstantUniforms();
    void createGeometries();
    void clearGeometries();
    void createCubeMap();
    void createIrradianceMap();
    void createShaderProgram(GlslProgram& program, string const& vertSource, string const& fragSource, char const* debugName);
private:
    const Window*   m_pWindow;
    GlslProgram     m_pbrProgram;
    GlslProgram     m_convertProgram;
    GlslProgram     m_irradianceProgram;
    GlslProgram     m_backgroundProgram;
    PerDrawData     m_sphere;
    PerDrawData     m_cube;
    GLTexture       m_hdrTexture;
    GLTexture       m_cubeMapTexture;
    GLTexture       m_irradianceTexture;
    GLFramebuffer   m_framebuffer;
};

} } // namespace pbr::gl

