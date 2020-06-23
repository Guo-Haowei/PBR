#pragma once
#include "core/Renderer.h"
#include "D3dPrerequisites.h"
#include "D3d11Helpers.h"

namespace pbr { namespace d3d11 {

class D3d11Renderer : public Renderer
{
public:
    D3d11Renderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render(const Camera& camera) override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void createDevice();
    void createSwapchain();
    void createRenderTarget(const Extent2i& extent);
    void cleanupRenderTarget();
    void compileShaders();
private:
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
    ComPtr<ID3D11Buffer>            m_vertexBuffer;
    ComPtr<ID3D11Buffer>            m_constantBuffer;
    // buffers
    PerFrameBuffer                  m_perFrameBuffer;
    // rasterizer
    ComPtr<ID3D11RasterizerState>   m_rasterizer;
};

} } // namespace pbr::d3d11
