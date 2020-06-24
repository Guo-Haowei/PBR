#include "base/Error.h"
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
    // draw spheres
    m_pbrProgram.use();
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
    m_envProgram.use();
    if (camera.IsDirty())
    {
        m_envProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_envProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
    }

    glBindVertexArray(m_envMap.vao);
    glDrawElements(GL_TRIANGLES, m_envMap.indexCount, GL_UNSIGNED_INT, 0);
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
    auto image = utility::ReadHDRImage(DATA_DIR "hdr/ballroom.hdr");
    m_hdrTexture = CreateHDRTexture(image);
    free(image.buffer.pData);
}

// shaders
#define PBR_VERT "pbr.vert"
#define PBR_FRAG "pbr.frag"
#define ENV_VERT "env.vert"
#define ENV_FRAG "env.frag"

void GLRendererImpl::compileShaders()
{
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::pbr_vert_c_str);
        string fragSource = string(generated::pbr_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR PBR_VERT);
        string fragSource = utility::ReadAsciiFile(GLSL_DIR PBR_FRAG);
#endif
        SHADER_COMPILING_START_INFO(PBR_VERT);
        GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
        SHADER_COMPILING_END_INFO(PBR_VERT);
        SHADER_COMPILING_START_INFO(PBR_FRAG);
        GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
        SHADER_COMPILING_END_INFO(PBR_FRAG);
        m_pbrProgram = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);

        // upload constant buffers
        uploadConstantUniforms();
    }
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::env_vert_c_str);
        string fragSource = string(generated::env_frag_c_str);
#else
        string vertSource = utility::ReadAsciiFile(GLSL_DIR ENV_VERT);
        string fragSource = utility::ReadAsciiFile(GLSL_DIR ENV_FRAG);
#endif
        SHADER_COMPILING_START_INFO(ENV_VERT);
        GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
        SHADER_COMPILING_END_INFO(ENV_VERT);
        SHADER_COMPILING_START_INFO(ENV_FRAG);
        GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
        SHADER_COMPILING_END_INFO(ENV_FRAG);
        m_envProgram = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);
    }
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
        m_envMap.indexCount = static_cast<uint32_t>(3 * cube.indices.size());
        glGenVertexArrays(1, &m_envMap.vao);
        glBindVertexArray(m_envMap.vao);
        glGenBuffers(2, &m_envMap.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_envMap.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indices.size() * sizeof(uvec3), cube.indices.data(), GL_STATIC_DRAW);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, m_envMap.vbo);
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

    m_envProgram.use();
    m_envProgram.setUniform("u_env_map", 0);
}

} } // namespace pbr::gl

