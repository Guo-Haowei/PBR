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
    void createCubeMapTexture();
    void createShaderProgram(GlslProgram& program, string const& vertSource, string const& fragSource, char const* debugName);
private:
    const Window*   m_pWindow;
    GlslProgram     m_pbrProgram;
    GlslProgram     m_envProgram;
    GlslProgram     m_backgroundProgram;
    PerDrawData     m_sphere;
    PerDrawData     m_cube;
    GLTexture       m_hdrTexture;
    GLTexture       m_cubeMapTexture;
};

} } // namespace pbr::gl

