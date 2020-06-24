#include "base/Error.h"
#include "GLRendererImpl.h"
#include "GLPrerequisites.h"
#include "Utility.h"
#include "Mesh.h"
#include "Scene.h"
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
    // drawing
    m_pbrProgram.use();
    if (camera.IsDirty())
    {
        m_pbrProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrixGl());
        m_pbrProgram.setUniform("u_view_pos", camera.GetViewPos());
    }

    const int size = 7;
    glDrawElementsInstanced(GL_TRIANGLES, m_sphere.indexCount, GL_UNSIGNED_INT, 0, size * size);

    // for (const mat4& m : transforms)
    // {
    //     m_pbrProgram.setUniform("u_per_draw.transform", m);
    //     glDrawElements(GL_TRIANGLES, m_sphere.indexCount, GL_UNSIGNED_INT, 0);
    // }
}

void GLRendererImpl::Resize(const Extent2i& extent)
{
}

void GLRendererImpl::Finalize()
{
    // delete resources
    m_pbrProgram.destroy();
    destroySphereBuffers();
}

void GLRendererImpl::PrepareGpuResources()
{
    // compile shaders
    compileShaders();

    // buffer
    createSphereBuffers();
    // glBindVertexArray(0);
}

// shaders
#define PBR_VERT "pbr.vert"
#define PBR_FRAG "pbr.frag"

void GLRendererImpl::compileShaders()
{
    {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        string vertSource = string(generated::pbr_vert_c_str);
        string fragSource = string(generated::pbr_frag_c_str);
#else
        string vertSource = utility::readAsciiFile(GLSL_DIR PBR_VERT);
        string fragSource = utility::readAsciiFile(GLSL_DIR PBR_FRAG);
#endif
        SHADER_COMPILING_START_INFO(PBR_VERT);
        GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
        SHADER_COMPILING_END_INFO(PBR_VERT);
        SHADER_COMPILING_START_INFO(PBR_FRAG);
        GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
        SHADER_COMPILING_END_INFO(PBR_FRAG);
        m_pbrProgram = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);

        // upload constant buffers
        uploadLightUniforms();
    }
}

void GLRendererImpl::createSphereBuffers()
{
    m_sphere.indexCount = static_cast<uint32_t>(3 * g_sphere.indices.size());
    glGenVertexArrays(1, &m_sphere.vao);
    glBindVertexArray(m_sphere.vao);
    glGenBuffers(2, &m_sphere.vbo);
    // ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_sphere.indices.size() * sizeof(uvec3), g_sphere.indices.data(), GL_STATIC_DRAW);
    // positions
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere.vbo);
    glBufferData(GL_ARRAY_BUFFER, g_sphere.vertices.size() * sizeof(Vertex), g_sphere.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
}

void GLRendererImpl::destroySphereBuffers()
{
    glDeleteVertexArrays(1, &m_sphere.vao);
    glDeleteVertexArrays(2, &m_sphere.vbo);
}

void GLRendererImpl::uploadLightUniforms()
{
    m_pbrProgram.use();
    for (size_t i = 0; i < g_lights.size(); ++i)
    {
        const string light = "u_lights[" + std::to_string(i) + "].";
        m_pbrProgram.setUniform(light + "position", g_lights[i].position);
        m_pbrProgram.setUniform(light + "color", g_lights[i].color);
    }
}

} } // namespace pbr::gl

