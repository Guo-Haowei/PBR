#include "D3d11Renderer.h"
#include "D3dDebug.h"
#include "D3d11Helpers.h"
#include "Window.h"
#include <iostream>

namespace pbr {

D3d11Renderer::D3d11Renderer(const Window* pWindow) : Renderer(pWindow)
{
}

void D3d11Renderer::Initialize()
{
    m_hwnd = glfwGetWin32Window(m_pWindow->GetInternalWindow());
    createDevice();
    createSwapchain();
    createRenderTarget();
}

void D3d11Renderer::Render()
{
    // clear
    static const float clearColor[4] = { 0.4f, 0.3f, 0.3f, 1.0f };
    // set render target
    m_deviceContext->OMSetRenderTargets(1, m_immediateRenderTarget.GetAddressOf(), NULL);
    // shaders
    m_deviceContext->VSSetShader(m_vert.Get(), NULL, 0);
    m_deviceContext->PSSetShader(m_pixel.Get(), NULL, 0);
    // set viewport
    const Extent2i& extent = m_pWindow->GetFrameBufferExtent();
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<float>(extent.width);
    viewport.Height = static_cast<float>(extent.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);
    // input assembly
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    // draw
    m_deviceContext->ClearRenderTargetView(m_immediateRenderTarget.Get(), clearColor);
    m_deviceContext->Draw(3, 0);
    m_swapChain->Present(0, 0); // m_swapChain->Present(1, 0);
    // present
}

void D3d11Renderer::Finalize()
{

}

void D3d11Renderer::createDevice()
{
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT createDeviceFlags = 0;
#ifdef PBR_DEBUG
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

    D3D_THROW_IF_FAILED(hr, "Failed to create d3d11 device");

    D3D_THROW_IF_FAILED(
        m_device->QueryInterface(__uuidof(IDXGIDevice),(void**)m_dxgiDevice.GetAddressOf()),
        "Failed to query IDXGIDevice");

    D3D_THROW_IF_FAILED(
        m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)m_dxgiAdapter.GetAddressOf()),
        "Failed to query IDXGIAdapter");

    D3D_THROW_IF_FAILED(
        m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)m_dxgiFactory.GetAddressOf()),
        "Failed to query IDXGIFactory");
}

void D3d11Renderer::createSwapchain()
{
    // TODO: msaa
    DXGI_MODE_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
    bufferDesc.Width = 0;
    bufferDesc.Height = 0;
    bufferDesc.RefreshRate = { 60, 1 };
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.BufferDesc = bufferDesc;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = m_hwnd;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_THROW_IF_FAILED(m_dxgiFactory->CreateSwapChain(m_device.Get(), &desc, m_swapChain.GetAddressOf()),
        "Failed to create swap chain");
}

void D3d11Renderer::DumpGraphicsCardInfo()
{
    DXGI_ADAPTER_DESC desc {};
    D3D_THROW_IF_FAILED(m_dxgiAdapter->GetDesc(&desc), "Failed to get adapter description");

    std::wcout << "Graphics Card:     " << desc.Description << std::endl;
}

void D3d11Renderer::PrepareGpuResources()
{
    compileShaders();

    // TODO: refactor
    HRESULT hr;

    // input layout
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(vec3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        inputElementDescs,
        ARRAYSIZE(inputElementDescs),
        m_vertBlob->GetBufferPointer(),
        m_vertBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()
    );

    D3D_THROW_IF_FAILED(hr, "Failed to create input layout");

    // vertex buffer
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(g_triangle);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA srData;
    ZeroMemory(&srData, sizeof(D3D11_SUBRESOURCE_DATA));
    srData.pSysMem = g_triangle;
    D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &srData, m_vertexBuffer.GetAddressOf()),
        "Failed to create vertex buffer");
}

void D3d11Renderer::Resize(const Extent2i& extent)
{
    cleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, extent.width, extent.height, DXGI_FORMAT_UNKNOWN, 0);
    createRenderTarget();
}

void D3d11Renderer::createRenderTarget()
{
    ComPtr<ID3D11Texture2D> backbuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    HRESULT hr = m_device->CreateRenderTargetView(backbuffer.Get(), NULL, m_immediateRenderTarget.GetAddressOf());
    D3D_THROW_IF_FAILED(hr, "Failed to create immediate render target");
}

void D3d11Renderer::cleanupRenderTarget()
{
    if (m_immediateRenderTarget != nullptr)
        m_immediateRenderTarget->Release();
}

// shaders
#define PBR_SHADER "pbr.hlsl"

void D3d11Renderer::compileShaders()
{
    HRESULT hr;
    {
        SHADER_COMPILING_START_INFO(PBR_SHADER ".vert");
        HlslShader::CompileShader(HLSL_DIR PBR_SHADER, "vs_main", "vs_5_0", m_vertBlob);
        hr = m_device->CreateVertexShader(
            m_vertBlob->GetBufferPointer(),
            m_vertBlob->GetBufferSize(),
            NULL,
            m_vert.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        SHADER_COMPILING_END_INFO(PBR_SHADER ".vert");
        SHADER_COMPILING_START_INFO(PBR_SHADER ".pixel");
        ComPtr<ID3DBlob> pixelBlob;
        HlslShader::CompileShader(HLSL_DIR PBR_SHADER, "ps_main", "ps_5_0", pixelBlob);
        hr = m_device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            m_pixel.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(PBR_SHADER ".pixel");

        m_deviceContext->VSSetShader(m_vert.Get(), 0, 0);
        m_deviceContext->PSSetShader(m_pixel.Get(), 0, 0);
    }
}

} // namespace pbr

