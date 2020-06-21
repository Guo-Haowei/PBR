#pragma once
#include "Renderer.h"
#include "D3DPrerequisites.h"
#include <d3d11.h>

namespace pbr {

class D3D11Renderer : public Renderer
{
public:
    D3D11Renderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void createDevice();
    void createSwapchain();
    void createRenderTarget();
    void cleanupRenderTarget();
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
};

} // namespace pbr
