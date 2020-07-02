#include "base/Error.h"
#include "core/Renderer.h"
#include "GLRendererImpl.h"
#include "GLPrerequisites.h"
#include "Utility.h"
#include "Mesh.h"
#include "Scene.h"
#include "Global.h"
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include "shaders.generated.h"
#endif

namespace pbr { namespace gl {

GLRendererImpl::GLRendererImpl(const Window* pWindow) : m_pWindow(pWindow)
{
}

void GLRendererImpl::Initialize()
{

#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
    if (gladLoadGL() == 0)
        THROW_EXCEPTION("GLAD: Failed to load glad functions");
#endif

#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void GLRendererImpl::DumpGraphicsCardInfo()
{
    // cout << "Vendor:            " << glGetString(GL_VENDOR) << endl;
    cout << "Graphics Card:     " << glGetString(GL_RENDERER) << endl;
    cout << "Version OpenGL:    " << glGetString(GL_VERSION) << endl;
    cout << "Version GLSL:      " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void GLRendererImpl::Render(const Camera& camera)
{
    // set viewport
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    glViewport(0, 0, extent.width, extent.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTexture.handle);

    glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTexture.handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilteredTexture.handle);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    // draw spheres
    m_pbrProgram.use();
    m_pbrProgram.setUniform("u_irradiance_map", 0);
    if (camera.IsDirty())
    {
        m_pbrProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
        m_pbrProgram.setUniform("u_view_pos", camera.GetViewPos());
    }
    glBindVertexArray(m_sphere.vao);
    const int size = 7;
    glDrawElementsInstanced(GL_TRIANGLES, m_sphere.indexCount, GL_UNSIGNED_INT, 0, size * size);

    // draw cube map
    m_backgroundProgram.use();
    if (camera.IsDirty())
    {
        m_backgroundProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_backgroundProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
    }

    m_backgroundProgram.setUniform("u_env_map", 1);

    glBindVertexArray(m_cube.vao);
    glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
}

void GLRendererImpl::Resize(const Extent2i& extent)
{
}

void GLRendererImpl::Finalize()
{
    // delete resources
    m_pbrProgram.destroy();
    glDeleteTextures(1, &m_hdrTexture.handle);
    clearGeometries();
}

void GLRendererImpl::PrepareGpuResources()
{
    // compile shaders
    compileShaders();

    // buffer
    createGeometries();
    // glBindVertexArray(0);

    // load enviroment map
    auto image = utility::ReadHDRImage(DEFAULT_HDR_ENV_MAP);
    m_hdrTexture = CreateHDRTexture(image);
    free(image.buffer.pData);

    // convert HDR equirectuangular environment map to cubemap equivalent
    calculateCubemapMatrices();
    createFramebuffer();
    createCubeMap();
    createIrradianceMap();
    createPrefilteredMap();
    // create an irradiance cubemap, and solve diffuse integral by convolution to create an irradiance cube map
}

void GLRendererImpl::createFramebuffer()
{
    glGenFramebuffers(1, &m_framebuffer.fbo);
    glGenRenderbuffers(1, &m_framebuffer.rbo);
}

void GLRendererImpl::calculateCubemapMatrices()
{
    CubeCamera cubeCamera(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_cubeMapPerspective = cubeCamera.ProjectionMatrixGl();
    cubeCamera.ViewMatricesGl(m_cubeMapViews);
}

void GLRendererImpl::createCubeMap()
{
    m_cubeMapTexture = CreateEmptyCubeMap(Renderer::cubeMapRes, true);
    m_convertProgram.use();
    m_convertProgram.setUniform("u_env_map", 0);
    m_convertProgram.setUniform("u_per_frame.projection", m_cubeMapPerspective);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_framebuffer.rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Renderer::cubeMapRes, Renderer::cubeMapRes);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_framebuffer.rbo);

    glViewport(0, 0, Renderer::cubeMapRes, Renderer::cubeMapRes);
    for (int i = 0; i < 6; ++i)
    {
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
}

void GLRendererImpl::createIrradianceMap()
{
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

    for (int i = 0; i < 6; ++i)
    {
        m_irradianceProgram.setUniform("u_per_frame.view", m_cubeMapViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceTexture.handle, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(m_cube.vao);
        glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLRendererImpl::createPrefilteredMap()
{
    m_prefilteredTexture = CreateEmptyCubeMap(Renderer::prefilteredMapRes, Renderer::prefilteredMaxMipLevel);
    m_prefilterProgram.use();
    m_prefilterProgram.setUniform("u_env_map", 0);
    m_prefilterProgram.setUniform("u_per_frame.projection", m_cubeMapPerspective);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture.handle);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.fbo);

    unsigned int mipSize = Renderer::prefilteredMapRes;
    for (int mipLevel = 0; mipLevel < Renderer::prefilteredMaxMipLevel; ++mipLevel, mipSize = mipSize >> 1)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, m_framebuffer.rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
        glViewport(0, 0, mipSize, mipSize);

        float roughness = float(mipLevel) / float(Renderer::prefilteredMaxMipLevel - 1.0f);
        m_prefilterProgram.setUniform("u_roughness", roughness);
        for (int i = 0; i < 6; ++i)
        {
            m_prefilterProgram.setUniform("u_per_frame.view", m_cubeMapViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilteredTexture.handle, mipLevel);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(m_cube.vao);
            glDrawElements(GL_TRIANGLES, m_cube.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// shaders
void GLRendererImpl::createShaderProgram(GlslProgram& program, string const& vertSource, string const& fragSource, char const* debugName)
{
    SHADER_COMPILING_START_INFO(debugName);
    GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
    GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
    SHADER_COMPILING_END_INFO(debugName);
    program = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);
}

void GLRendererImpl::compileShaders()
{
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

    // upload constant buffers
    uploadConstantUniforms();
}

void GLRendererImpl::createGeometries()
{
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
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
    }
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
}

void GLRendererImpl::clearGeometries()
{
    glDeleteVertexArrays(1, &m_sphere.vao);
    glDeleteVertexArrays(2, &m_sphere.vbo);
}

void GLRendererImpl::uploadConstantUniforms()
{
    m_pbrProgram.use();
    for (size_t i = 0; i < g_lights.size(); ++i)
    {
        const string light = "u_lights[" + std::to_string(i) + "].";
        m_pbrProgram.setUniform(light + "position", g_lights[i].position);
        m_pbrProgram.setUniform(light + "color", g_lights[i].color);
    }

    m_convertProgram.use();
    m_convertProgram.setUniform("u_env_map", 0);
}

} } // namespace pbr::gl

