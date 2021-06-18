#include "GLRenderer.h"
#include "impl/GLRendererImpl.h"

namespace pbr {
namespace gl {

GLRenderer::GLRenderer(const Window* pWindow)
    : Renderer(pWindow)
    , impl(std::make_unique<GLRendererImpl>(pWindow)) {
}

void GLRenderer::Initialize() {
    impl->Initialize();
}

void GLRenderer::DumpGraphicsCardInfo() {
    impl->DumpGraphicsCardInfo();
}

void GLRenderer::Render(const Camera& camera) {
    impl->Render(camera);
}

void GLRenderer::Resize(const Extent2i& extent) {
    impl->Resize(extent);
}

void GLRenderer::Finalize() {
    impl->Finalize();
}

void GLRenderer::PrepareGpuResources() {
    impl->PrepareGpuResources();
}

}  // namespace gl
}  // namespace pbr
