#pragma once
#include "Renderer.h"
#include "D3dPrerequisites.h"

namespace pbr {

class D3d11Renderer : public Renderer
{
public:
    D3d11Renderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void PrepareGpuResources() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void createDevice();
    void createSwapchain();
    void createRenderTarget();
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
    // shaders
    ComPtr<ID3D11VertexShader>      m_vert;
    ComPtr<ID3D11PixelShader>       m_pixel;
    ComPtr<ID3DBlob>                m_vertBlob;
};

} // namespace pbr
