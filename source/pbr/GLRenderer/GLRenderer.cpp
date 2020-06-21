#include "GLRenderer.h"
#include "GLPrerequisites.h"
#include "GLDebugCb.h"
#include "Error.h"
#include "Window.h"
using std::cout;
using std::endl;

namespace pbr {

GLRenderer::GLRenderer(const Window* pWindow) : Renderer(pWindow)
{
}

void GLRenderer::Initialize()
{

#if TARGET_PLATFORM != PLATFORM_EMSCRIPTEN
    if (gladLoadGL() == 0)
        throw runtime_error("[Error][glad] failed to load glad functions");
#endif

#if PBR_GL_VERSION >= 430
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(internal::glDebugCb, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif
}

void GLRenderer::DumpGraphicsCardInfo()
{
    // cout << "Vendor:            " << glGetString(GL_VENDOR) << endl;
    cout << "Graphics Card:     " << glGetString(GL_RENDERER) << endl;
    cout << "Version OpenGL:    " << glGetString(GL_VERSION) << endl;
    cout << "Version GLSL:      " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void GLRenderer::Render()
{
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    glViewport(0, 0, extent.width, extent.height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLRenderer::Resize(const Extent2i& extent)
{
}

void GLRenderer::Finalize()
{
}

} // namespace pbr
