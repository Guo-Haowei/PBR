#include "D3d11RendererImpl.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "D3dDebug.h"
#include <iostream>
#include <cstddef> // offsetof
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace pbr { namespace d3d11 {

D3d11RendererImpl::D3d11RendererImpl(const Window* pWindow) : m_pWindow(pWindow)
{
}

void D3d11RendererImpl::Initialize()
{
    m_hwnd = glfwGetWin32Window(m_pWindow->GetInternalWindow());
    createDevice();
    createSwapchain();
    createRenderTarget(m_pWindow->GetFrameBufferExtent());
}

void D3d11RendererImpl::Finalize()
{

}

void D3d11RendererImpl::Render(const Camera& camera)
{
    // clear
    static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    // set render target
    m_deviceContext->OMSetRenderTargets(1, m_immediateRenderTarget.GetAddressOf(), m_immediateDepthStencil.Get());
    // shaders
    //m_deviceContext->VSSetShader(m_vert.Get(), NULL, 0);
    //m_deviceContext->PSSetShader(m_pixel.Get(), NULL, 0);
    // set perframe buffers
    if (camera.IsDirty())
    {
        m_perFrameBuffer.m_cache.view = camera.ViewMatrix();
        m_perFrameBuffer.m_cache.projection = convertProjection(camera.ProjectionMatrixD3d());
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        m_perFrameBuffer.Update(m_deviceContext);

        m_viewPositionBuffer.m_cache.view_position = camera.GetViewPos();
        m_viewPositionBuffer.PSSet(m_deviceContext, 1);
        m_viewPositionBuffer.Update(m_deviceContext);
    }
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
    m_deviceContext->IASetVertexBuffers(0, 1, m_sphere.vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_sphere.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // draw
    m_deviceContext->ClearRenderTargetView(m_immediateRenderTarget.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_immediateDepthStencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    const int size = 7;
    m_deviceContext->DrawIndexedInstanced(m_sphere.indexCount, size * size, 0, 0, 0);
    // present
    m_swapChain->Present(1, 0); // vsync
    // m_swapChain->Present(0, 0);
}

void D3d11RendererImpl::createDevice()
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

void D3d11RendererImpl::createSwapchain()
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
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = m_hwnd;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_THROW_IF_FAILED(m_dxgiFactory->CreateSwapChain(m_device.Get(), &desc, m_swapChain.GetAddressOf()),
        "Failed to create swap chain");
}

void D3d11RendererImpl::DumpGraphicsCardInfo()
{
    DXGI_ADAPTER_DESC desc {};
    D3D_THROW_IF_FAILED(m_dxgiAdapter->GetDesc(&desc), "Failed to get adapter description");

    std::wcout << "Graphics Card:     " << desc.Description << std::endl;
}

void D3d11RendererImpl::PrepareGpuResources()
{
    compileShaders();

    // TODO: refactor
    HRESULT hr;

    // input layout
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        inputElementDescs,
        ARRAYSIZE(inputElementDescs),
        m_vertBlob->GetBufferPointer(),
        m_vertBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()
    );

    D3D_THROW_IF_FAILED(hr, "Failed to create input layout");

    // sphere buffer
    createSphereBuffers();

    // constant buffer
    m_perFrameBuffer.Create(m_device);
    m_perDrawBuffer.Create(m_device);
    m_lightBuffer.Create(m_device);
    m_viewPositionBuffer.Create(m_device);
    memcpy(&m_lightBuffer.m_cache, &g_lights, m_lightBuffer.BufferSize());
    //m_deviceContext->PSSetShader(m_pixel.Get(), 0, 0);
    m_lightBuffer.PSSet(m_deviceContext, 0);
    m_lightBuffer.Update(m_deviceContext);

    // rasterizer
    {
        // D3D11_RASTERIZER_DESC desc;
        // ZeroMemory(&desc, sizeof(desc));
        // desc.FillMode = D3D11_FILL_SOLID;
        // desc.CullMode = D3D11_CULL_NONE;
        // m_device->CreateRasterizerState(&desc, m_rasterizer.GetAddressOf());
        // m_deviceContext->RSSetState(m_rasterizer.Get());
    }
}

void D3d11RendererImpl::Resize(const Extent2i& extent)
{
    cleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, extent.width, extent.height, DXGI_FORMAT_UNKNOWN, 0);
    createRenderTarget(extent);
}

void D3d11RendererImpl::createRenderTarget(const Extent2i& extent)
{
    ComPtr<ID3D11Texture2D> backbuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    HRESULT hr = m_device->CreateRenderTargetView(backbuffer.Get(), NULL, m_immediateRenderTarget.GetAddressOf());
    D3D_THROW_IF_FAILED(hr, "Failed to create immediate render target view");

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = extent.width;
    desc.Height = extent.height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthBuffer;

    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&desc, 0, depthBuffer.GetAddressOf()),
        "Failed to create depth buffer");
    D3D_THROW_IF_FAILED(m_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, m_immediateDepthStencil.GetAddressOf()),
        "Failed to create depth stencil view");
}

void D3d11RendererImpl::cleanupRenderTarget()
{
    if (m_immediateRenderTarget != nullptr)
        m_immediateRenderTarget->Release();
    if (m_immediateDepthStencil != nullptr)
        m_immediateDepthStencil->Release();
}

// shaders
#define PBR_VERT_SHADER "pbr.vert.hlsl"
#define PBR_PIXEL_SHADER "pbr.pixel.hlsl"

void D3d11RendererImpl::compileShaders()
{
    HRESULT hr;
    {
        SHADER_COMPILING_START_INFO(PBR_VERT_SHADER);
        HlslShader::CompileShader(HLSL_DIR PBR_VERT_SHADER, "vs_main", "vs_5_0", m_vertBlob);
        hr = m_device->CreateVertexShader(
            m_vertBlob->GetBufferPointer(),
            m_vertBlob->GetBufferSize(),
            NULL,
            m_vert.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        SHADER_COMPILING_END_INFO(PBR_VERT_SHADER);
        SHADER_COMPILING_START_INFO(PBR_PIXEL_SHADER);
        ComPtr<ID3DBlob> pixelBlob;
        HlslShader::CompileShader(HLSL_DIR PBR_PIXEL_SHADER, "ps_main", "ps_5_0", pixelBlob);
        hr = m_device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            m_pixel.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(PBR_PIXEL_SHADER);

        m_deviceContext->VSSetShader(m_vert.Get(), 0, 0);
        m_deviceContext->PSSetShader(m_pixel.Get(), 0, 0);
    }
}

void D3d11RendererImpl::createSphereBuffers()
{
    const auto sphere = CreateSphereMesh();
    m_sphere.indexCount = static_cast<uint32_t>(3 * sphere.indices.size());

    {
        // vertex buffer
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(Vertex) * sphere.vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data;
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = sphere.vertices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_sphere.vertexBuffer.GetAddressOf()),
            "Failed to create vertex buffer");
    }
    {
        // index buffer
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(uvec3) * sphere.indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data;
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = sphere.indices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_sphere.indexBuffer.GetAddressOf()),
            "Failed to create index buffer");
    }
}

} } // namespace pbr::d3d11
