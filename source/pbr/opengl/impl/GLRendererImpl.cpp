#include "GLRendererImpl.h"
#include "GLPrerequisites.h"
#include "Mesh.h"
#include "Paths.h"
#include "Scene.h"
#include "Utility.h"
#include "base/Error.h"
#include "core/Globals.h"
#include "core/Renderer.h"

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#include "shaders.generated.h"
#endif

namespace pbr {
namespace gl {

GLRendererImpl::GLRendererImpl(const Window* pWindow)
    : m_pWindow(pWindow) {
}

void GLRendererImpl::Initialize() {
#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
    if (gladLoadGL() == 0)
        THROW_EXCEPTION("GLAD: Failed to load glad functions");
#endif

#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl::DebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glDepthFunc(GL_LEQUAL);
    // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void GLRendererImpl::DumpGraphicsCardInfo() {
    // cout << "Vendor:            " << glGetString(GL_VENDOR) << endl;
    cout << "Graphics Card:     " << glGetString(GL_RENDERER) << endl;
    cout << "Version OpenGL:    " << glGetString(GL_VERSION) << endl;
    cout << "Version GLSL:      " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void GLRendererImpl::Render(const Camera& camera) {
    // set viewport
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    glViewport(0, 0, extent.width, extent.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw spheres
#if 0
    m_pbrProgram.use();
    m_pbrProgram.setUniform("u_debug", debug);
    if (camera.IsDirty())
    {
        m_pbrProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
        m_pbrProgram.setUniform("u_view_pos", camera.GetViewPos());
    }

    glBindVertexArray(m_sphere.vao);
    const int size = 5;
    glDrawElementsInstanced(GL_TRIANGLES, m_sphere.indexCount, GL_UNSIGNED_INT, 0, size * size);
#endif
    // draw model
    m_pbrModelProgram.use();
    m_pbrModelProgram.setUniform("u_debug", g_debug);
    if (camera.IsDirty()) {
        m_pbrModelProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrModelProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
        m_pbrModelProgram.setUniform("u_view_pos", camera.GetViewPos());
    }

    glBindVertexArray(m_model.vao);
    glDrawElements(GL_TRIANGLES, m_model.indexCount, GL_UNSIGNED_INT, 0);

    // draw cube map
    m_backgroundProgram.use();
    if (camera.IsDirty()) {
        m_backgroundProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_backgroundProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
    }

    glBindVertexArray(m_cube.vao);
    glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
}

void GLRendererImpl::Resize(const Extent2i& extent) {
}

void GLRendererImpl::Finalize() {
    // delete resources
    m_pbrProgram.destroy();
    m_pbrModelProgram.destroy();
    m_backgroundProgram.destroy();
    glDeleteTextures(1, &m_hdrTexture.handle);
    clearGeometries();
}

void GLRendererImpl::PrepareGpuResources() {
    // compile shaders
    compileShaders();

    // buffer
    createGeometries();

    // albedo metallic
    auto amImage = utility::ReadPng(g_model_dir + "AlbedoMetallic.png");
    m_albedoMetallicTexture = CreateTexture(amImage, GL_RGBA);
    free(amImage.buffer.pData);

    // normal roughness
    auto normalRoughnessImage = utility::ReadPng(g_model_dir + "NormalRoughness.png");
    m_normalRoughnessTexture = CreateTexture(normalRoughnessImage, GL_RGBA);
    free(normalRoughnessImage.buffer.pData);

    // emissive ao
    auto emissiveAO = utility::ReadPng(g_model_dir + "EmissiveAO.png");
    m_emissiveAOTexture = CreateTexture(emissiveAO, GL_RGBA);
    free(emissiveAO.buffer.pData);

    // load hdr texture
    auto envImage = utility::ReadHDRImage(g_env_map_path);
    m_hdrTexture = CreateTexture(envImage, GL_RGB32F);
    free(envImage.buffer.pData);
    // load brdf texture
    auto brdfImage = utility::ReadBrdfLUT(BRDF_LUT, Renderer::brdfLUTImageRes);
    m_brdfLUTTexture = CreateTexture(brdfImage, GL_RG16F);
    free(brdfImage.buffer.pData);

    // convert HDR equirectuangular environment map to cubemap equivalent
    calculateCubemapMatrices();
    createFramebuffer();
    createCubeMap();
    createIrradianceMap();
    createPrefilteredMap();

    // upload constant buffers
    uploadConstantUniforms();
}

void GLRendererImpl::createFramebuffer() {
    glGenFramebuffers(1, &m_framebuffer.fbo);
    glGenRenderbuffers(1, &m_framebuffer.rbo);
}

void GLRendererImpl::calculateCubemapMatrices() {
    CubeCamera cubeCamera(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_cubeMapPerspective = cubeCamera.ProjectionMatrixGl();
    cubeCamera.ViewMatricesGl(m_cubeMapViews);
}

void GLRendererImpl::createCubeMap() {
    m_cubeMapTexture = CreateEmptyCubeMap(Renderer::cubeMapRes, true);
    m_convertProgram.use();
    m_convertProgram.setUniform("u_env_map", 0);
    m_convertProgram.setUniform("u_per_frame.projection", m_cubeMapPerspective);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture.handle);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_framebuffer.rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Renderer::cubeMapRes, Renderer::cubeMapRes);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_framebuffer.rbo);

    glViewport(0, 0, Renderer::cubeMapRes, Renderer::cubeMapRes);
    for (int i = 0; i < 6; ++i) {
        m_convertProgram.setUniform("u_per_frame.view", m_cubeMapViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubeMapTexture.handle, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(m_cube.vao);
        glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_convertProgram.destroy();
}

void GLRendererImpl::createIrradianceMap() {
    m_irradianceTexture = CreateEmptyCubeMap(Renderer::irradianceMapRes);
    m_irradianceProgram.use();
    m_irradianceProgram.setUniform("u_env_map", 0);
    m_irradianceProgram.setUniform("u_per_frame.projection", m_cubeMapPerspective);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_framebuffer.rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Renderer::irradianceMapRes, Renderer::irradianceMapRes);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_framebuffer.rbo);

    glViewport(0, 0, Renderer::irradianceMapRes, Renderer::irradianceMapRes);

    for (int i = 0; i < 6; ++i) {
        m_irradianceProgram.setUniform("u_per_frame.view", m_cubeMapViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceTexture.handle, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(m_cube.vao);
        glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_irradianceProgram.destroy();
}

void GLRendererImpl::createPrefilteredMap() {
    m_specularTexture = CreateEmptyCubeMap(Renderer::specularMapRes, Renderer::specularMapMipLevels);
    m_prefilterProgram.use();
    m_prefilterProgram.setUniform("u_env_map", 0);
    m_prefilterProgram.setUniform("u_per_frame.projection", m_cubeMapPerspective);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fbo);

    unsigned int mipSize = Renderer::specularMapRes;
    for (int mipLevel = 0; mipLevel < Renderer::specularMapMipLevels; ++mipLevel, mipSize = mipSize >> 1) {
        glBindRenderbuffer(GL_RENDERBUFFER, m_framebuffer.rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
        glViewport(0, 0, mipSize, mipSize);

        float roughness = float(mipLevel) / float(Renderer::specularMapMipLevels - 1.0f);
        m_prefilterProgram.setUniform("u_roughness", roughness);
        for (int i = 0; i < 6; ++i) {
            m_prefilterProgram.setUniform("u_per_frame.view", m_cubeMapViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_specularTexture.handle, mipLevel);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(m_cube.vao);
            glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_prefilterProgram.destroy();
}

// shaders
void GLRendererImpl::createShaderProgram(GlslProgram& program, string const& vertSource, string const& fragSource, char const* debugName) {
    SHADER_COMPILING_START_INFO(debugName);
    GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
    GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
    SHADER_COMPILING_END_INFO(debugName);
    program = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);
}

void GLRendererImpl::compileShaders() {
    // pbr
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::pbr_vert_c_str);
        string fragSource = string(generated::pbr_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "pbr.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "pbr.frag");
#endif
        createShaderProgram(m_pbrProgram, vertSource, fragSource, "PBR Program");
    }
    // pbr_model
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::pbr_model_vert_c_str);
        string fragSource = string(generated::pbr_model_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "pbr_model.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "pbr_model.frag");
#endif
        createShaderProgram(m_pbrModelProgram, vertSource, fragSource, "PBR Model Program");
    }
    // convert cubemap
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::cubemap_vert_c_str);
        string fragSource = string(generated::to_cubemap_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "cubemap.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "to_cubemap.frag");
#endif
        createShaderProgram(m_convertProgram, vertSource, fragSource, "Convert Program");
    }
    // irradiance map
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::cubemap_vert_c_str);
        string fragSource = string(generated::irradiance_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "cubemap.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "irradiance.frag");
#endif
        createShaderProgram(m_irradianceProgram, vertSource, fragSource, "Irradiance Program");
    }
    // prefiltered map
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::cubemap_vert_c_str);
        string fragSource = string(generated::prefilter_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "cubemap.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "prefilter.frag");
#endif
        createShaderProgram(m_prefilterProgram, vertSource, fragSource, "Prefilter Program");
    }
    // background
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::background_vert_c_str);
        string fragSource = string(generated::background_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR "background.vert");
        string fragSource = utility::ReadAsciiFile(GLSL_DIR "background.frag");
#endif
        createShaderProgram(m_backgroundProgram, vertSource, fragSource, "Background Program");
    }
}

void GLRendererImpl::createGeometries() {
    {
        // cube
        const auto cube = CreateCubeMesh(1.0f);
        m_cube.indexCount = static_cast<uint32_t>(3 * cube.indices.size());
        glGenVertexArrays(1, &m_cube.vao);
        glBindVertexArray(m_cube.vao);
        glGenBuffers(2, &m_cube.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cube.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indices.size() * sizeof(uvec3), cube.indices.data(), GL_STATIC_DRAW);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, m_cube.vbo);
        glBufferData(GL_ARRAY_BUFFER, cube.vertices.size() * sizeof(vec3), cube.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
        glEnableVertexAttribArray(0);
    }
    {
        // sphere
        const auto sphere = CreateSphereMesh();
        m_sphere.indexCount = static_cast<uint32_t>(3 * sphere.indices.size());
        glGenVertexArrays(1, &m_sphere.vao);
        glBindVertexArray(m_sphere.vao);
        glGenBuffers(2, &m_sphere.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(uvec3), sphere.indices.data(), GL_STATIC_DRAW);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, m_sphere.vbo);
        glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(Vertex), sphere.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
    }
    {
        // load model
        auto model = utility::LoadModel(g_model_dir.c_str());

        m_model.indexCount = static_cast<uint32_t>(3 * model.indices.size());
        glGenVertexArrays(1, &m_model.vao);
        glBindVertexArray(m_model.vao);
        glGenBuffers(2, &m_model.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_model.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(uvec3), model.indices.data(), GL_STATIC_DRAW);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, m_model.vbo);
        glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(TexturedVertex), model.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, uv));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, tangent));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, bitangent));
        glEnableVertexAttribArray(4);
    }
}

void GLRendererImpl::clearGeometries() {
    glDeleteVertexArrays(1, &m_sphere.vao);
    glDeleteVertexArrays(2, &m_sphere.vbo);
}

void GLRendererImpl::uploadConstantUniforms() {
    m_pbrProgram.use();
    // lighting
    for (size_t i = 0; i < g_lights.size(); ++i) {
        const string light = "u_lights[" + std::to_string(i) + "].";
        m_pbrProgram.setUniform(light + "position", g_lights[i].position);
        m_pbrProgram.setUniform(light + "color", g_lights[i].color);
    }

    // textures
    m_pbrProgram.setUniform("u_irradiance_map", 1);
    m_pbrProgram.setUniform("u_specular_map", 2);
    m_pbrProgram.setUniform("u_brdf_lut", 3);

    m_pbrModelProgram.use();
    // lighting
    for (size_t i = 0; i < g_lights.size(); ++i) {
        const string light = "u_lights[" + std::to_string(i) + "].";
        m_pbrModelProgram.setUniform(light + "position", g_lights[i].position);
        m_pbrModelProgram.setUniform(light + "color", g_lights[i].color);
    }

    m_pbrModelProgram.setUniform("u_per_draw.transform", g_transform);

    // textures
    m_pbrModelProgram.setUniform("u_irradiance_map", 1);
    m_pbrModelProgram.setUniform("u_specular_map", 2);
    m_pbrModelProgram.setUniform("u_brdf_lut", 3);
    m_pbrModelProgram.setUniform("u_albedoMetallic", 4);
    m_pbrModelProgram.setUniform("u_normalRoughness", 5);
    m_pbrModelProgram.setUniform("u_emissiveAO", 6);

    m_backgroundProgram.use();
    m_backgroundProgram.setUniform("u_env_map", 0);

    glActiveTexture(GL_TEXTURE0);  // background
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    glActiveTexture(GL_TEXTURE1);  // irradiance
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTexture.handle);

    glActiveTexture(GL_TEXTURE2);  // prefiltered texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_specularTexture.handle);

    glActiveTexture(GL_TEXTURE3);  // brdf
    glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture.handle);

    glActiveTexture(GL_TEXTURE4);  // albedo + metallic
    glBindTexture(GL_TEXTURE_2D, m_albedoMetallicTexture.handle);

    glActiveTexture(GL_TEXTURE5);  // normal + roughness
    glBindTexture(GL_TEXTURE_2D, m_normalRoughnessTexture.handle);

    glActiveTexture(GL_TEXTURE6);  // emissive + ao
    glBindTexture(GL_TEXTURE_2D, m_emissiveAOTexture.handle);
}

}  // namespace gl
}  // namespace pbr
