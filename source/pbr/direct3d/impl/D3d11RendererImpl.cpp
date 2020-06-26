#include "D3d11RendererImpl.h"
#include "core/Window.h"
#include "core/Camera.h"
#include "core/Renderer.h"
#include "Utility.h"
#include "D3dDebug.h"
#include <iostream>
#include <cstddef> // offsetof
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace pbr { namespace d3d11 {

static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

D3d11RendererImpl::D3d11RendererImpl(const Window* pWindow) : m_pWindow(pWindow)
{
}

void D3d11RendererImpl::Initialize()
{
    m_hwnd = glfwGetWin32Window(m_pWindow->GetInternalWindow());
    createDevice();
    createSwapchain();
    createImmediateRenderTarget(m_pWindow->GetFrameBufferExtent());
    createCubeMapRenderTarget(Extent2i(Renderer::CubeMapRes, Renderer::CubeMapRes));
}

void D3d11RendererImpl::Finalize()
{

}

void D3d11RendererImpl::Render(const Camera& camera)
{
    // set render target
    m_deviceContext->OMSetRenderTargets(1, m_immediate.rtv.GetAddressOf(), m_immediate.dsv.Get());
    // set viewport
    setViewport(m_pWindow->GetFrameBufferExtent());
    // clear
    m_deviceContext->ClearRenderTargetView(m_immediate.rtv.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_immediate.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    // set primitive topology

    // update shared constant buffer
    if (camera.IsDirty())
    {
        m_perFrameBuffer.m_cache.view = camera.ViewMatrix();
        m_perFrameBuffer.m_cache.projection = camera.ProjectionMatrixD3d();
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        m_viewPositionBuffer.m_cache.view_position = camera.GetViewPos();
        m_viewPositionBuffer.Update(m_deviceContext);
        m_viewPositionBuffer.PSSet(m_deviceContext, 1);
    }
    // draw spheres
    {
        // set shader
        m_deviceContext->VSSetShader(m_pbrVert.Get(), NULL, 0);
        m_deviceContext->PSSetShader(m_pbrPixel.Get(), NULL, 0);
        if (camera.IsDirty())
        {
            m_perFrameBuffer.VSSet(m_deviceContext, 1);
            m_viewPositionBuffer.PSSet(m_deviceContext, 1);
        }
        // set input layout
        m_deviceContext->IASetInputLayout(m_sphereLayout.Get());
        // set vertex/index buffer
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_deviceContext->IASetVertexBuffers(0, 1, m_sphere.vertexBuffer.GetAddressOf(), &stride, &offset);
        m_deviceContext->IASetIndexBuffer(m_sphere.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        const int size = 7;
        // draw
        m_deviceContext->DrawIndexedInstanced(m_sphere.indexCount, size * size, 0, 0, 0);
    }
    // draw cube
    {
        // set shader
        m_deviceContext->VSSetShader(m_backgroundVert.Get(), NULL, 0);
        m_deviceContext->PSSetShader(m_backgroundPixel.Get(), NULL, 0);
        m_deviceContext->PSSetShaderResources(0, 1, m_environment.srv.GetAddressOf());
        m_deviceContext->PSSetSamplers(0, 1, m_hdrTexture->m_sampler.GetAddressOf());
        if (camera.IsDirty())
        {
            m_perFrameBuffer.VSSet(m_deviceContext, 1);
            m_viewPositionBuffer.PSSet(m_deviceContext, 1);
        }
        // set input layout
        m_deviceContext->IASetInputLayout(m_cubeLayout.Get());
        // set vertex/index buffer
        UINT stride = sizeof(vec3);
        UINT offset = 0;
        m_deviceContext->IASetVertexBuffers(0, 1, m_cube.vertexBuffer.GetAddressOf(), &stride, &offset);
        m_deviceContext->IASetIndexBuffer(m_cube.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        // draw
        m_deviceContext->DrawIndexed(m_cube.indexCount, 0, 0);
    }
    // present
    m_swapChain->Present(1, 0); // vsync
    // m_swapChain->Present(0, 0);
}

void D3d11RendererImpl::setViewport(const Extent2i& extent)
{
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<float>(extent.width);
    viewport.Height = static_cast<float>(extent.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);
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
    // shaders
    compileShaders();
    // geometries
    createGeometries();

    // hdr texture
    auto image = utility::ReadHDRImage(DEFAULT_HDR_ENV_MAP);
    m_hdrTexture.reset(CreateHDRTexture(m_device, image));

    // constant buffer
    m_perFrameBuffer.Create(m_device);
    m_perDrawBuffer.Create(m_device);
    m_lightBuffer.Create(m_device);
    m_viewPositionBuffer.Create(m_device);
    memcpy(&m_lightBuffer.m_cache, &g_lights, m_lightBuffer.BufferSize());
    m_deviceContext->PSSetShader(m_pbrPixel.Get(), 0, 0);
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

    // set depth function to less equal
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc {};
        dsDesc.DepthEnable = true;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.StencilEnable = false;

        D3D_THROW_IF_FAILED(m_device->CreateDepthStencilState(&dsDesc, m_depthStencilState.GetAddressOf()),
            "Failed to create depth stencil state");

        m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
    }

    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // render environment map once gpu resources are ready
    renderToEnvironmentMap();
}

void D3d11RendererImpl::Resize(const Extent2i& extent)
{
    cleanupImmediateRenderTarget();
    m_swapChain->ResizeBuffers(0, extent.width, extent.height, DXGI_FORMAT_UNKNOWN, 0);
    createImmediateRenderTarget(extent);
}

void D3d11RendererImpl::createImmediateRenderTarget(const Extent2i& extent)
{
    ComPtr<ID3D11Texture2D> backbuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    HRESULT hr = m_device->CreateRenderTargetView(backbuffer.Get(), NULL, m_immediate.rtv.GetAddressOf());
    D3D_THROW_IF_FAILED(hr, "Failed to create immediate render target view");

    D3D11_TEXTURE2D_DESC desc {};
    desc.Width = extent.width;
    desc.Height = extent.height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthBuffer;

    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&desc, 0, depthBuffer.GetAddressOf()),
        "Failed to create depth buffer");

    D3D_THROW_IF_FAILED(m_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, m_immediate.dsv.GetAddressOf()),
        "Failed to create depth stencil view");
}

void D3d11RendererImpl::createCubeMapRenderTarget(const Extent2i& extent)
{
    D3D11_TEXTURE2D_DESC textureDesc {};
    textureDesc.Width = extent.width;
    textureDesc.Height = extent.height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&textureDesc, NULL, m_environment.cubeBuffer.GetAddressOf()),
        "Failed to create cube buffer");

    D3D11_TEXTURE2D_DESC depthStencilDesc {};
    depthStencilDesc.Width = extent.width;
    depthStencilDesc.Height = extent.height;
    depthStencilDesc.MipLevels = depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&depthStencilDesc, 0, m_environment.depthBuffer.GetAddressOf()),
        "Failed to create depth buffer");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    D3D_THROW_IF_FAILED(m_device->CreateShaderResourceView(m_environment.cubeBuffer.Get(), &srvDesc, m_environment.srv.GetAddressOf()),
        "Failed to create shader resource view");

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.MipSlice = 0;
    rtvDesc.Texture2DArray.ArraySize = 1;
    for (int i = 0; i < m_environment.rtvs.size(); ++i)
    {
        rtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i, 1);
        D3D_THROW_IF_FAILED(m_device->CreateRenderTargetView(m_environment.cubeBuffer.Get(), &rtvDesc, m_environment.rtvs[i].GetAddressOf()),
            "Failed to create render target view " + std::to_string(i));
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc {};

    D3D_THROW_IF_FAILED(m_device->CreateDepthStencilView(m_environment.depthBuffer.Get(), nullptr, m_environment.dsv.GetAddressOf()),
        "Failed to create depth stencil view");

}

void D3d11RendererImpl::renderToEnvironmentMap()
{
    // set input layout
    m_deviceContext->IASetInputLayout(m_cubeLayout.Get());
    // set viewport
    setViewport(Extent2i(Renderer::CubeMapRes, Renderer::CubeMapRes));
    UINT stride = sizeof(vec3), offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_cube.vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_cube.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // set shader
    m_deviceContext->VSSetShader(m_envVert.Get(), NULL, 0);
    m_deviceContext->PSSetShader(m_envPixel.Get(), NULL, 0);
    m_deviceContext->PSSetShaderResources(0, 1, m_hdrTexture->m_shaderResourceView.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_hdrTexture->m_sampler.GetAddressOf());

    CubeCamera camera(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_perFrameBuffer.m_cache.projection = camera.ProjectionMatrixD3d();
    array<mat4, 6> viewMatrices;
    camera.ViewMatricesD3d(viewMatrices);

    for (int i = 0; i < 6; ++i)
    {
        m_deviceContext->OMSetRenderTargets(1, m_environment.rtvs[i].GetAddressOf(), m_environment.dsv.Get());
        m_deviceContext->ClearRenderTargetView(m_environment.rtvs[i].Get(), clearColor);
        m_deviceContext->ClearDepthStencilView(m_environment.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // update shared constant buffer
        m_perFrameBuffer.m_cache.view = viewMatrices[i];
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);

        // draw
        m_deviceContext->DrawIndexed(m_cube.indexCount, 0, 0);
    }
}

void D3d11RendererImpl::cleanupImmediateRenderTarget()
{
    if (m_immediate.rtv != nullptr)
        m_immediate.rtv->Release();
    if (m_immediate.dsv != nullptr)
        m_immediate.dsv->Release();
}

// shaders
#define PBR_VERT_SHADER "pbr.vert.hlsl"
#define PBR_PIXEL_SHADER "pbr.pixel.hlsl"
#define ENV_VERT_SHADER "env.vert.hlsl"
#define ENV_PIXEL_SHADER "env.pixel.hlsl"
#define BG_VERT_SHADER "background.vert.hlsl"
#define BG_PIXEL_SHADER "background.pixel.hlsl"

void D3d11RendererImpl::compileShaders()
{
    {
        // pbr
        SHADER_COMPILING_START_INFO(PBR_VERT_SHADER);
        HlslShader::CompileShader(HLSL_DIR PBR_VERT_SHADER, "vs_main", "vs_5_0", m_pbrVertShaderBlob);
        HRESULT hr = m_device->CreateVertexShader(
            m_pbrVertShaderBlob->GetBufferPointer(),
            m_pbrVertShaderBlob->GetBufferSize(),
            NULL,
            m_pbrVert.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        SHADER_COMPILING_END_INFO(PBR_VERT_SHADER);
        SHADER_COMPILING_START_INFO(PBR_PIXEL_SHADER);
        ComPtr<ID3DBlob> pixelBlob;
        HlslShader::CompileShader(HLSL_DIR PBR_PIXEL_SHADER, "ps_main", "ps_5_0", pixelBlob);
        hr = m_device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            m_pbrPixel.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(PBR_PIXEL_SHADER);
    }
    {
        // environment mapping
        SHADER_COMPILING_START_INFO(ENV_VERT_SHADER);
        HlslShader::CompileShader(HLSL_DIR ENV_VERT_SHADER, "vs_main", "vs_5_0", m_cubeVertShaderBlob);
        HRESULT hr = m_device->CreateVertexShader(
            m_cubeVertShaderBlob->GetBufferPointer(),
            m_cubeVertShaderBlob->GetBufferSize(),
            NULL,
            m_envVert.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        SHADER_COMPILING_END_INFO(ENV_VERT_SHADER);
        SHADER_COMPILING_START_INFO(ENV_PIXEL_SHADER);
        ComPtr<ID3DBlob> pixelBlob;
        HlslShader::CompileShader(HLSL_DIR ENV_PIXEL_SHADER, "ps_main", "ps_5_0", pixelBlob);
        hr = m_device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            m_envPixel.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(ENV_PIXEL_SHADER);
    }
    {
        // background visualization
        SHADER_COMPILING_START_INFO(BG_VERT_SHADER);
        ComPtr<ID3DBlob> vertBlob;
        HlslShader::CompileShader(HLSL_DIR BG_VERT_SHADER, "vs_main", "vs_5_0", vertBlob);
        HRESULT hr = m_device->CreateVertexShader(
            vertBlob->GetBufferPointer(),
            vertBlob->GetBufferSize(),
            NULL,
            m_backgroundVert.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        SHADER_COMPILING_END_INFO(BG_VERT_SHADER);
        SHADER_COMPILING_START_INFO(BG_PIXEL_SHADER);
        ComPtr<ID3DBlob> pixelBlob;
        HlslShader::CompileShader(HLSL_DIR BG_PIXEL_SHADER, "ps_main", "ps_5_0", pixelBlob);
        hr = m_device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            m_backgroundPixel.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(BG_PIXEL_SHADER);
    }
}

void D3d11RendererImpl::createGeometries()
{
    // sphere
    const auto sphere = CreateSphereMesh();
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
        m_sphere.indexCount = static_cast<uint32_t>(3 * sphere.indices.size());
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
    {
        // input layout
        D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HRESULT hr = m_device->CreateInputLayout(
            inputElementDescs,
            ARRAYSIZE(inputElementDescs),
            m_pbrVertShaderBlob->GetBufferPointer(),
            m_pbrVertShaderBlob->GetBufferSize(),
            m_sphereLayout.GetAddressOf()
        );

        D3D_THROW_IF_FAILED(hr, "Failed to create sphere input layout");
    }
    // cube
    const auto cube = CreateCubeMesh(1.0f);
    {
        // vertex buffer
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(vec3) * cube.vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data;
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = cube.vertices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_cube.vertexBuffer.GetAddressOf()),
            "Failed to create vertex buffer");
    }
    {
        // index buffer
        m_cube.indexCount = static_cast<uint32_t>(3 * cube.indices.size());
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(uvec3) * cube.indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data;
        ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
        data.pSysMem = cube.indices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_cube.indexBuffer.GetAddressOf()),
            "Failed to create index buffer");
    }
    {
        // input layout
        D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HRESULT hr = m_device->CreateInputLayout(
            inputElementDescs,
            ARRAYSIZE(inputElementDescs),
            m_cubeVertShaderBlob->GetBufferPointer(),
            m_cubeVertShaderBlob->GetBufferSize(),
            m_cubeLayout.GetAddressOf()
        );

        D3D_THROW_IF_FAILED(hr, "Failed to create cube input layout");
    }
}

} } // namespace pbr::d3d11
