#include "D3D11Renderer.h"
#include "D3DDebug.h"
#include "Window.h"
#include <iostream>
#pragma comment(lib, "d3d11.lib")

namespace pbr {

D3D11Renderer::D3D11Renderer(const Window* pWindow) : Renderer(pWindow)
{
}

void D3D11Renderer::Initialize()
{
    m_hwnd = glfwGetWin32Window(m_pWindow->GetInternalWindow());
    createDevice();
    createSwapchain();
    createRenderTarget();
}

void D3D11Renderer::Render()
{
    static const float clearColor[4] = { 0.4f, 0.3f, 0.3f, 1.0f };
    m_deviceContext->ClearRenderTargetView(m_immediateRenderTarget.Get(), clearColor);
    // const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    m_swapChain->Present(0, 0);
    // m_swapChain->Present(1, 0);
}

void D3D11Renderer::Finalize()
{

}

void D3D11Renderer::createDevice()
{
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // pAdapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,                          // HMODULE Software
        createDeviceFlags,
        &featureLevel,              // in feature levels
        1,                          // number of feature levels
        D3D11_SDK_VERSION,
        m_device.GetAddressOf(),
        nullptr,                    // out feature levels
        m_deviceContext.GetAddressOf());

    THROW_IF_NOT_OK(hr, "Failed to create d3d11 device");

    THROW_IF_NOT_OK(
        m_device->QueryInterface(__uuidof(IDXGIDevice),(void**)m_dxgiDevice.GetAddressOf()),
        "Failed to query IDXGIDevice");

    THROW_IF_NOT_OK(
        m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)m_dxgiAdapter.GetAddressOf()),
        "Failed to query IDXGIAdapter");

    THROW_IF_NOT_OK(
        m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)m_dxgiFactory.GetAddressOf()),
        "Failed to query IDXGIFactory");
}

void D3D11Renderer::createSwapchain()
{
    // TODO: msaa
    DXGI_SWAP_CHAIN_DESC desc {};
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = m_hwnd;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;

    THROW_IF_NOT_OK(m_dxgiFactory->CreateSwapChain(m_device.Get(), &desc, m_swapChain.GetAddressOf()),
        "Failed to create swap chain");
}

void D3D11Renderer::DumpGraphicsCardInfo()
{
    DXGI_ADAPTER_DESC desc {};
    THROW_IF_NOT_OK(m_dxgiAdapter->GetDesc(&desc), "Failed to get adapter description");

    std::wcout << "Graphics Card:     " << desc.Description << std::endl;
}

void D3D11Renderer::Resize(const Extent2i& extent)
{
    cleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, extent.width, extent.height, DXGI_FORMAT_UNKNOWN, 0);
    createRenderTarget();
}

void D3D11Renderer::createRenderTarget()
{
    ComPtr<ID3D11Texture2D> backbuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    HRESULT hr = m_device->CreateRenderTargetView(backbuffer.Get(), NULL, m_immediateRenderTarget.GetAddressOf());
    THROW_IF_NOT_OK(hr, "Failed to create immediate render target");
}

void D3D11Renderer::cleanupRenderTarget()
{
    if (m_immediateRenderTarget != nullptr)
        m_immediateRenderTarget->Release();
}

} // namespace pbr

