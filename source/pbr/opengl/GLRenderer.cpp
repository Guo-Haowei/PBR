#include "base/Error.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "GLRenderer.h"
#include "GLPrerequisites.h"
#include "Utility.h"
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
    glClear(GL_COLOR_BUFFER_BIT);
    // drawing
    m_pbrProgram.use();
    if (camera.IsDirty())
    {
        m_pbrProgram.setUniform("u_per_frame.view", camera.ViewMatrix());
        m_pbrProgram.setUniform("u_per_frame.projection", camera.ProjectionMatrix());
    }
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLRenderer::Resize(const Extent2i& extent)
{
}

void GLRenderer::Finalize()
{
    // delete resources
}

void GLRenderer::PrepareGpuResources()
{
    // compile shaders
    compileShaders();

    // buffer
    glGenVertexArrays(1, &m_triangleVao);
    glGenBuffers(1, &m_triangleBuffer);
    glBindVertexArray(m_triangleVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangle), g_triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
    glEnableVertexAttribArray(1);
    // glBindVertexArray(0);
}

// shaders
#define PBR_VERT "pbr.vert"
#define PBR_FRAG "pbr.frag"

void GLRenderer::compileShaders()
{
    {
        SHADER_COMPILING_START_INFO(PBR_VERT);
        string vertSource = utility::readAsciiFile(GLSL_DIR PBR_VERT);
        GLuint vertexShaderHandle = GlslProgram::createShaderFromString(vertSource, GL_VERTEX_SHADER);
        SHADER_COMPILING_END_INFO(PBR_VERT);
        SHADER_COMPILING_START_INFO(PBR_FRAG);
        string fragSource = utility::readAsciiFile(GLSL_DIR PBR_FRAG);
        GLuint fragmentShaderHandle = GlslProgram::createShaderFromString(fragSource, GL_FRAGMENT_SHADER);
        SHADER_COMPILING_END_INFO(PBR_FRAG);
        m_pbrProgram = GlslProgram::create(vertexShaderHandle, fragmentShaderHandle);
    }
}

} } // namespace pbr::gl
