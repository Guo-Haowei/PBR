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
    void createRenderTarget(const Extent2i& extent);
    void cleanupRenderTarget();
    void compileShaders();
    void createSphereBuffers();
private:
    const Window*                   m_pWindow;
    HWND                            m_hwnd;
    ComPtr<ID3D11Device>            m_device;
    ComPtr<ID3D11DeviceContext>     m_deviceContext;
    ComPtr<IDXGIDevice>             m_dxgiDevice;
    ComPtr<IDXGIAdapter>            m_dxgiAdapter;
    ComPtr<IDXGIFactory>            m_dxgiFactory;
    ComPtr<IDXGISwapChain>          m_swapChain;
    // TODO: refactor
    // render target
    ComPtr<ID3D11RenderTargetView>  m_immediateRenderTarget;
    ComPtr<ID3D11DepthStencilView>  m_immediateDepthStencil;
    // shaders
    ComPtr<ID3D11VertexShader>      m_vert;
    ComPtr<ID3D11PixelShader>       m_pixel;
    ComPtr<ID3DBlob>                m_vertBlob;
    // input
    ComPtr<ID3D11InputLayout>       m_inputLayout;
    ComPtr<ID3D11Buffer>            m_constantBuffer;
    // buffers
    PerFrameBuffer                  m_perFrameBuffer;
    PerDrawBuffer                   m_perDrawBuffer;
    LightBuffer                     m_lightBuffer;
    ViewPositionBuffer              m_viewPositionBuffer;
    // rasterizer
    ComPtr<ID3D11RasterizerState>   m_rasterizer;
    PerDrawData                     m_sphere;
    // reverse depth
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
};

} } // namespace pbr::d3d11
