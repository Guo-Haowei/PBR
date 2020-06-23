#include "base/Error.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "GLRenderer.h"
#include "GLPrerequisites.h"
#include "Utility.h"
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include "shaders.generated.h"
#endif
#include <cstddef> // offsetof
using std::cout;
using std::endl;

namespace pbr { namespace gl {

GLRenderer::GLRenderer(const Window* pWindow) : Renderer(pWindow)
{
}

void GLRenderer::Initialize()
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

void GLRenderer::DumpGraphicsCardInfo()
{
    // cout << "Vendor:            " << glGetString(GL_VENDOR) << endl;
    cout << "Graphics Card:     " << glGetString(GL_RENDERER) << endl;
    cout << "Version OpenGL:    " << glGetString(GL_VERSION) << endl;
    cout << "Version GLSL:      " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void GLRenderer::Render(const Camera& camera)
{
    // set viewport
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    glViewport(0, 0, extent.width, extent.height);
    // clear screen
    glClearColor(0.3f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // drawing
    m_pbrProgram.use();
    if (camera.IsDirty())
    {
        m_pbrProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrix());
    }
    glDrawElements(GL_TRIANGLES, m_sphere.indexCount, GL_UNSIGNED_INT, 0);
}

void GLRenderer::Resize(const Extent2i& extent)
{
}

void GLRenderer::Finalize()
{
    // delete resources
    m_pbrProgram.destroy();
    destroySphereBuffers();
}

void GLRenderer::PrepareGpuResources()
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

void GLRenderer::compileShaders()
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
    }
}

void GLRenderer::createSphereBuffers()
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

void GLRenderer::destroySphereBuffers()
{
    glDeleteVertexArrays(1, &m_sphere.vao);
    glDeleteVertexArrays(2, &m_sphere.vbo);
}

} } // namespace pbr::gl
