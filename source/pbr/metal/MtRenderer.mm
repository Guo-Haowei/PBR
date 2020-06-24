#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

#import "base/Error.h"
#import "core/Window.h"
#import "MtRenderer.h"
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <simd/simd.h>

id<MTLDevice> device;

namespace pbr { namespace mt {

MtRenderer::MtRenderer(const Window* pWindow) : Renderer(pWindow) { }

void MtRenderer::Initialize()
{
    device = MTLCreateSystemDefaultDevice();
    if (!device)
        THROW_EXCEPTION("metal: Failed to create default device");

    NSWindow* nswin = glfwGetCocoaWindow(m_pWindow->GetInternalWindow());
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;
    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.languageVersion = MTLLanguageVersion1_1;

    NSError* compileError;
    id<MTLLibrary> lib = [device newLibraryWithSource:
       @"#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "vertex float4 v_simple(\n"
        "    constant float4* in  [[buffer(0)]],\n"
        "    uint             vid [[vertex_id]])\n"
        "{\n"
        "    return in[vid];\n"
        "}\n"
        "fragment float4 f_simple(\n"
        "    float4 in [[stage_in]])\n"
        "{\n"
        "    return float4(1, 0, 0, 1);\n"
        "}\n"
       options:compileOptions error:&compileError];
    if (!lib)
    {
        // const char *cfilename=[stringobject UTF8String];
        NSLog(@"can't create library: %@", compileError);
    }
}

void MtRenderer::DumpGraphicsCardInfo()
{

}

void MtRenderer::PrepareGpuResources()
{

}

void MtRenderer::Render(const Camera& camera)
{

}

void MtRenderer::Resize(const Extent2i& extent)
{

}

void MtRenderer::Finalize()
{

}

} } // namespace pbr::mt
