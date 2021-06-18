#import "MtRendererImpl.h"
#import "Mesh.h"
#import "MtHelpers.h"
#import "base/Error.h"
#import "core/Window.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#import <Cocoa/Cocoa.h>
#import <GLFW/glfw3.h>
#import <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

id<MTLDevice> g_device;
CAMetalLayer* g_pLayer;
id<MTLLibrary> g_lib;
id<MTLCommandQueue> g_commandQueue;
id<MTLRenderPipelineState> g_renderPipelineState;
id<MTLBuffer> g_triangleBuffer;

namespace pbr {
namespace mt {

MtRendererImpl::MtRendererImpl(const Window* pWindow)
    : m_pWindow(pWindow) {}

void MtRendererImpl::Initialize() {
    g_device = MTLCreateSystemDefaultDevice();
    if (!g_device)
        THROW_EXCEPTION("metal: Failed to create default device");

    NSWindow* nswin = glfwGetCocoaWindow(m_pWindow->GetInternalWindow());
    g_pLayer = [CAMetalLayer layer];
    g_pLayer.device = g_device;
    g_pLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = g_pLayer;
    nswin.contentView.wantsLayer = YES;
    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.languageVersion = MTLLanguageVersion1_1;

    NSString* shaderSource = [NSString stringWithContentsOfFile:
                                           METAL_DIR @"pbr.metal"
                                                       encoding:NSUTF8StringEncoding
                                                          error:NULL];

    NSError* compileError;
    g_lib = [g_device newLibraryWithSource:shaderSource options:compileOptions error:&compileError];
    if (!g_lib) {
        NSString* error = [NSString stringWithFormat:@"Failed to create library:\n\t%@", compileError];
        MT_THROW_NS_STRING(error);
    }

    id<MTLFunction> vs = [g_lib newFunctionWithName:@"vs_main"];
    assert(vs);
    id<MTLFunction> fs = [g_lib newFunctionWithName:@"fs_main"];
    assert(fs);

    g_commandQueue = [g_device newCommandQueue];
    assert(g_commandQueue);

    MTLRenderPipelineDescriptor* renderPipelineDesc = [MTLRenderPipelineDescriptor new];
    renderPipelineDesc.vertexFunction = vs;
    renderPipelineDesc.fragmentFunction = fs;
    renderPipelineDesc.colorAttachments[0].pixelFormat = g_pLayer.pixelFormat;
    g_renderPipelineState = [g_device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:NULL];
    assert(g_renderPipelineState);

    // buffer
    // g_triangleBuffer = [g_device newBufferWithBytes:g_triangle length:sizeof(g_triangle) options:MTLCPUCacheModeWriteCombined];
    // assert(g_triangleBuffer);
}

void MtRendererImpl::DumpGraphicsCardInfo() {
}

void MtRendererImpl::PrepareGpuResources() {
}

void MtRendererImpl::Render(const Camera& camera) {
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    g_pLayer.drawableSize = CGSizeMake(extent.width, extent.height);
    id<CAMetalDrawable> drawable = [g_pLayer nextDrawable];
    assert(drawable);

    id<MTLCommandBuffer> commandBuffer = [g_commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor new];
    MTLRenderPassColorAttachmentDescriptor* renderTarget = renderPassDesc.colorAttachments[0];
    renderTarget.texture = drawable.texture;
    renderTarget.loadAction = MTLLoadActionClear;
    renderTarget.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    renderTarget.storeAction = MTLStoreActionStore;
    id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];

    [renderCommandEncoder setRenderPipelineState:g_renderPipelineState];
    [renderCommandEncoder setVertexBytes:(vec4[]) {
                                             { 0, 0.5, 0, 1 },
                                             { -0.5, -0.5, 0, 1 },
                                             { 0.5, -0.5, 0, 1 },
                                         }
                                  length:3 * sizeof(vec4)
                                 atIndex:0];
    // [renderCommandEncoder setVertexBuffer:g_triangleBuffer offset:0 atIndex:0];
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

    [renderCommandEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

void MtRendererImpl::Resize(const Extent2i& extent) {
}

void MtRendererImpl::Finalize() {
}

}
}  // namespace pbr::mt
