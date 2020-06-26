#pragma once
#include "core/Renderer.h"
#include "D3dPrerequisites.h"
#include "D3d11Helpers.h"

namespace pbr { namespace d3d11 {

class D3d11RendererImpl
{
public:
    D3d11RendererImpl(const Window* pWindow);
    void Initialize();
    void DumpGraphicsCardInfo();
    void PrepareGpuResources();
    void Render(const Camera& camera);
    void Resize(const Extent2i& extent);
    void Finalize();
private:
    void createDevice();
    void createSwapchain();
    void createImmediateRenderTarget(const Extent2i& extent);
    void createCubeMapRenderTarget(const Extent2i& extent);
    void cleanupImmediateRenderTarget();
    void compileShaders();
    void createGeometries();
    void setViewport(const Extent2i& extent);
    void renderToEnvironmentMap();
    void renderCube();
    void renderSpheres();
private:
    const Window*                   m_pWindow;
    HWND                            m_hwnd;
    ComPtr<ID3D11Device>            m_device;
    ComPtr<ID3D11DeviceContext>     m_deviceContext;
    ComPtr<IDXGIDevice>             m_dxgiDevice;
    ComPtr<IDXGIAdapter>            m_dxgiAdapter;
    ComPtr<IDXGIFactory>            m_dxgiFactory;
    ComPtr<IDXGISwapChain>          m_swapChain;
    // render target
    ImmediateRenderTarget           m_immediate;
    CubeMapRenderTarget             m_environment;
    // shaders
    HlslProgram                     m_pbrProgram;
    HlslProgram                     m_envProgram;
    HlslProgram                     m_backgroundProgram;
    // input
    ComPtr<ID3D11Buffer>            m_constantBuffer;
    // buffers
    PerFrameBuffer                  m_perFrameBuffer;
    PerDrawBuffer                   m_perDrawBuffer;
    LightBuffer                     m_lightBuffer;
    ViewPositionBuffer              m_viewPositionBuffer;
    ComPtr<ID3D11InputLayout>       m_cubeLayout;
    PerDrawData                     m_cube;
    ComPtr<ID3D11InputLayout>       m_sphereLayout;
    PerDrawData                     m_sphere;
    // rasterizer
    ComPtr<ID3D11RasterizerState>   m_rasterizer;
    // reverse depth
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    // textures
    unique_ptr<Texture2D>           m_hdrTexture;
};

} } // namespace pbr::d3d11
