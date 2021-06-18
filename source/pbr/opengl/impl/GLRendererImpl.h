#pragma once
#include "GLHelpers.h"
#include "GLPrerequisites.h"
#include "core/Camera.h"
#include "core/Window.h"

namespace pbr {
namespace gl {

class GLRendererImpl {
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
    void createPrefilteredMap();
    void calculateCubemapMatrices();
    void createShaderProgram(GlslProgram& program, string const& vertSource, string const& fragSource, char const* debugName);

   private:
    const Window* m_pWindow;
    GlslProgram m_pbrProgram;
    GlslProgram m_pbrModelProgram;
    GlslProgram m_convertProgram;
    GlslProgram m_irradianceProgram;
    GlslProgram m_prefilterProgram;
    GlslProgram m_backgroundProgram;
    PerDrawData m_sphere;
    PerDrawData m_cube;
    PerDrawData m_model;
    GLTexture m_hdrTexture;
    GLTexture m_brdfLUTTexture;
    GLTexture m_cubeMapTexture;
    GLTexture m_irradianceTexture;
    GLTexture m_specularTexture;
    GLTexture m_albedoMetallicTexture;
    GLTexture m_normalRoughnessTexture;
    GLTexture m_emissiveAOTexture;
    GLFramebuffer m_framebuffer;

    mat4 m_cubeMapPerspective;
    array<mat4, 6> m_cubeMapViews;
};

}  // namespace gl
}  // namespace pbr
