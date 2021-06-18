#include "D3d11RendererImpl.h"
#include <cstddef>  // offsetof
#include <iostream>
#include "D3dDebug.h"
#include "Utility.h"
#include "core/Camera.h"
#include "core/Globals.h"
#include "core/Renderer.h"
#include "core/Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "Paths.h"

namespace pbr {
namespace d3d11 {

static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

D3d11RendererImpl::D3d11RendererImpl(const Window* pWindow)
    : m_pWindow(pWindow) {
}

void D3d11RendererImpl::Initialize() {
    m_hwnd = glfwGetWin32Window(m_pWindow->GetInternalWindow());
    createDevice();
    createSwapchain();
    createImmediateRenderTarget(m_pWindow->GetFrameBufferExtent());
}

void D3d11RendererImpl::Finalize() {
}

void D3d11RendererImpl::renderSpheres() {
    // set input layout
    m_deviceContext->IASetInputLayout(m_meshLayout.Get());
    // set vertex/index buffer
    UINT stride = sizeof(Vertex), offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_sphere.vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_sphere.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    const int size = 5;
    // draw
    m_deviceContext->DrawIndexedInstanced(m_sphere.indexCount, size * size, 0, 0, 0);
}

void D3d11RendererImpl::renderModel() {
    // set input layout
    m_deviceContext->IASetInputLayout(m_texturedMeshLayout.Get());
    // set vertex/index buffer
    UINT stride = sizeof(TexturedVertex), offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_model.vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_model.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // draw
    m_deviceContext->DrawIndexed(m_model.indexCount, 0, 0);
}

void D3d11RendererImpl::renderCube() {
    // set input layout
    m_deviceContext->IASetInputLayout(m_cubeLayout.Get());
    // set buffers
    UINT stride = sizeof(vec3), offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_cube.vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_cube.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // draw
    m_deviceContext->DrawIndexed(m_cube.indexCount, 0, 0);
}

void D3d11RendererImpl::setSrvAndSamplers() {
    m_deviceContext->PSSetShaderResources(0, 1, m_environmentMap.srv.GetAddressOf());
    m_deviceContext->PSSetShaderResources(1, 1, m_brdfLUTSrv.GetAddressOf());
    m_deviceContext->PSSetShaderResources(2, 1, m_specularMap.srv.GetAddressOf());
    m_deviceContext->PSSetShaderResources(3, 1, m_irradianceMap.srv.GetAddressOf());
    m_deviceContext->PSSetShaderResources(4, 1, m_albedoMetallic.GetAddressOf());
    m_deviceContext->PSSetShaderResources(5, 1, m_normalRoughness.GetAddressOf());
    m_deviceContext->PSSetShaderResources(6, 1, m_emissiveAO.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    m_deviceContext->PSSetSamplers(1, 1, m_samplerLod.GetAddressOf());
}

void D3d11RendererImpl::Render(const Camera& camera) {
    // set render target
    m_deviceContext->OMSetRenderTargets(1, m_immediate.rtv.GetAddressOf(), m_immediate.dsv.Get());
    // set viewport
    const Extent2i extent = m_pWindow->GetFrameBufferExtent();
    setViewport(extent.width, extent.height);
    // clear
    m_deviceContext->ClearRenderTargetView(m_immediate.rtv.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_immediate.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    // set primitive topology

    // update shared constant buffer
    if (camera.IsDirty()) {
        m_perFrameBuffer.m_cache.view = camera.ViewMatrix();
        m_perFrameBuffer.m_cache.projection = camera.ProjectionMatrixD3d();
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
    }

    m_viewPositionBuffer.m_cache.view_position = vec3(camera.GetViewPos());
    m_viewPositionBuffer.m_cache.padding = g_debug;
    m_viewPositionBuffer.Update(m_deviceContext);
    m_viewPositionBuffer.PSSet(m_deviceContext, 1);

    // render spheres
    // m_pbrProgram.set(m_deviceContext);

    if (camera.IsDirty()) {
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        m_viewPositionBuffer.PSSet(m_deviceContext, 1);
    }

    // renderSpheres();

    m_pbrModelProgram.set(m_deviceContext);
    renderModel();

    // render background
    m_backgroundProgram.set(m_deviceContext);
    if (camera.IsDirty()) {
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        m_viewPositionBuffer.PSSet(m_deviceContext, 1);
    }

    renderCube();

    // present
    m_swapChain->Present(1, 0);  // vsync
    // m_swapChain->Present(0, 0);
}

void D3d11RendererImpl::setViewport(int width, int height) {
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height < 0 ? width : height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);
}

void D3d11RendererImpl::createDevice() {
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT createDeviceFlags = 0;
#ifdef PBR_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif  // DEBUG

    HRESULT hr = D3D11CreateDevice(
        nullptr,  // pAdapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,  // HMODULE Software
        createDeviceFlags,
        &featureLevel,  // in feature levels
        1,              // number of feature levels
        D3D11_SDK_VERSION,
        m_device.GetAddressOf(),
        nullptr,  // out feature levels
        m_deviceContext.GetAddressOf());

    D3D_THROW_IF_FAILED(hr, "Failed to create d3d11 device");

    D3D_THROW_IF_FAILED(
        m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)m_dxgiDevice.GetAddressOf()),
        "Failed to query IDXGIDevice");

    D3D_THROW_IF_FAILED(
        m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)m_dxgiAdapter.GetAddressOf()),
        "Failed to query IDXGIAdapter");

    D3D_THROW_IF_FAILED(
        m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)m_dxgiFactory.GetAddressOf()),
        "Failed to query IDXGIFactory");
}

void D3d11RendererImpl::createSwapchain() {
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

void D3d11RendererImpl::DumpGraphicsCardInfo() {
    DXGI_ADAPTER_DESC desc {};
    D3D_THROW_IF_FAILED(m_dxgiAdapter->GetDesc(&desc), "Failed to get adapter description");

    std::wcout << "Graphics Card:     " << desc.Description << std::endl;
}

void D3d11RendererImpl::PrepareGpuResources() {
    // shaders
    compileShaders();
    // geometries
    createGeometries();

    // sampler
    createSampler();

    // load hdr texture
    auto envImage = utility::ReadHDRImage(g_env_map_path);
    createTexture2D(m_hdrSrv, envImage, DXGI_FORMAT_R32G32B32_FLOAT);
    free(envImage.buffer.pData);
    // load brdf texture
    auto brdfImage = utility::ReadBrdfLUT(BRDF_LUT, Renderer::brdfLUTImageRes);
    createTexture2D(m_brdfLUTSrv, brdfImage, DXGI_FORMAT_R32G32_FLOAT);
    free(brdfImage.buffer.pData);
    // load albedo
    auto albedoMetallicImage = utility::ReadPng(g_model_dir + "AlbedoMetallic.png");
    createTexture2D(m_albedoMetallic, albedoMetallicImage, DXGI_FORMAT_R8G8B8A8_UNORM);
    free(albedoMetallicImage.buffer.pData);
    // normal roughness
    auto normalRoughnessImage = utility::ReadPng(g_model_dir + "NormalRoughness.png");
    createTexture2D(m_normalRoughness, normalRoughnessImage, DXGI_FORMAT_R8G8B8A8_UNORM);
    free(normalRoughnessImage.buffer.pData);
    // emissive ao
    auto emissiveAOImage = utility::ReadPng(g_model_dir + "EmissiveAO.png");
    createTexture2D(m_emissiveAO, emissiveAOImage, DXGI_FORMAT_R8G8B8A8_UNORM);
    free(emissiveAOImage.buffer.pData);

    // constant buffer
    m_perFrameBuffer.Create(m_device);
    m_perDrawBuffer.Create(m_device);
    m_lightBuffer.Create(m_device);
    m_viewPositionBuffer.Create(m_device);

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
    // render environment map only once
    calculateCubemapMatrices();
    createCubemap(m_environmentMap, Renderer::cubeMapRes, Renderer::specularMapMipLevels, true);
    renderToEnvironmentMap();
    createCubemap(m_irradianceMap, Renderer::irradianceMapRes);
    renderToIrradianceMap();
    createCubemap(m_specularMap, Renderer::specularMapRes, Renderer::specularMapMipLevels);
    renderToSpecularMap();

    uploadConstantBuffer();
    setSrvAndSamplers();
}

void D3d11RendererImpl::createSampler() {
    D3D11_SAMPLER_DESC samplerDesc {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW =
        D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    D3D_THROW_IF_FAILED(m_device->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf()),
                        "Failed to create sampler satete");

    // samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.MaxLOD = float(Renderer::specularMapMipLevels) - 1.0f;

    D3D_THROW_IF_FAILED(m_device->CreateSamplerState(&samplerDesc, m_samplerLod.GetAddressOf()),
                        "Failed to create sampler satete");
}

void D3d11RendererImpl::uploadConstantBuffer() {
    memcpy(&m_lightBuffer.m_cache, &g_lights, m_lightBuffer.BufferSize());
    // m_deviceContext->PSSetShader(m_pbrProgram.pixelShader.Get(), 0, 0);
    m_lightBuffer.PSSet(m_deviceContext, 0);
    m_lightBuffer.Update(m_deviceContext);

    m_perDrawBuffer.m_cache.transform = g_transform;
    m_perDrawBuffer.VSSet(m_deviceContext, 0);
    m_perDrawBuffer.Update(m_deviceContext);
}

void D3d11RendererImpl::Resize(const Extent2i& extent) {
    cleanupImmediateRenderTarget();
    m_swapChain->ResizeBuffers(0, extent.width, extent.height, DXGI_FORMAT_UNKNOWN, 0);
    createImmediateRenderTarget(extent);
}

void D3d11RendererImpl::createImmediateRenderTarget(const Extent2i& extent) {
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

void D3d11RendererImpl::createCubemap(CubemapTexture& target, int res, int mipLevels, bool genMips) {
    D3D11_TEXTURE2D_DESC textureDesc {};
    textureDesc.Width = textureDesc.Height = res;
    textureDesc.MipLevels = mipLevels;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    if (genMips)
        textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&textureDesc, NULL, target.cubeBuffer.GetAddressOf()),
                        "Failed to create cube buffer");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = mipLevels;
    D3D_THROW_IF_FAILED(m_device->CreateShaderResourceView(target.cubeBuffer.Get(), &srvDesc, target.srv.GetAddressOf()),
                        "Failed to create shader resource view");
}

void D3d11RendererImpl::renderToEnvironmentMap() {
    // set viewport
    setViewport(Renderer::cubeMapRes);

    // set shader
    m_convertProgram.set(m_deviceContext);
    m_deviceContext->PSSetShaderResources(0, 1, m_hdrSrv.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    m_perFrameBuffer.m_cache.projection = m_cubeMapPerspective;

    for (int face = 0; face < 6; ++face) {
        // temporary render target view
        ComPtr<ID3D11RenderTargetView> rtv;
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};
        rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.ArraySize = 1;
        rtvDesc.Texture2DArray.FirstArraySlice = face;
        D3D_THROW_IF_FAILED(m_device->CreateRenderTargetView(m_environmentMap.cubeBuffer.Get(), &rtvDesc, rtv.GetAddressOf()),
                            "Failed to create render target view");

        m_deviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);
        m_deviceContext->ClearRenderTargetView(rtv.Get(), clearColor);

        // update shared constant buffer
        m_perFrameBuffer.m_cache.view = m_cubeMapViews[face];
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        renderCube();
    }

    // unbound srv and rtv
    ID3D11ShaderResourceView* nullSrv = nullptr;
    ID3D11RenderTargetView* nullRtv = nullptr;
    m_deviceContext->PSSetShaderResources(0, 1, &nullSrv);
    m_deviceContext->OMSetRenderTargets(1, &nullRtv, nullptr);

    m_deviceContext->GenerateMips(m_environmentMap.srv.Get());
}

void D3d11RendererImpl::renderToIrradianceMap() {
    // set viewport
    setViewport(Renderer::irradianceMapRes);

    m_irradianceProgram.set(m_deviceContext);
    m_deviceContext->PSSetShaderResources(0, 1, m_environmentMap.srv.GetAddressOf());
    m_deviceContext->PSSetSamplers(1, 1, m_samplerLod.GetAddressOf());

    m_perFrameBuffer.m_cache.projection = m_cubeMapPerspective;

    for (int face = 0; face < 6; ++face) {
        // temporary render target view
        ComPtr<ID3D11RenderTargetView> rtv;
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};
        rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.ArraySize = 1;
        rtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, face, 1);
        D3D_THROW_IF_FAILED(m_device->CreateRenderTargetView(m_irradianceMap.cubeBuffer.Get(), &rtvDesc, rtv.GetAddressOf()),
                            "Failed to create render target view");

        m_deviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);
        m_deviceContext->ClearRenderTargetView(rtv.Get(), clearColor);

        // update shared constant buffer
        m_perFrameBuffer.m_cache.view = m_cubeMapViews[face];
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);
        renderCube();
    }

    // unbound srv and rtv
    ID3D11ShaderResourceView* nullSrv = nullptr;
    ID3D11RenderTargetView* nullRtv = nullptr;
    m_deviceContext->PSSetShaderResources(0, 1, &nullSrv);
    m_deviceContext->OMSetRenderTargets(1, &nullRtv, nullptr);
}

void D3d11RendererImpl::renderToSpecularMap() {
    m_prefilterProgram.set(m_deviceContext);
    m_deviceContext->PSSetShaderResources(0, 1, m_environmentMap.srv.GetAddressOf());
    m_deviceContext->PSSetSamplers(1, 1, m_samplerLod.GetAddressOf());
    // m_deviceContext->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    m_perFrameBuffer.m_cache.projection = m_cubeMapPerspective;

    FourFloatsBuffer m_specularRoughnessBuffer;
    m_specularRoughnessBuffer.Create(m_device);

    for (int face = 0; face < 6; ++face) {
        // update shared constant buffer
        m_perFrameBuffer.m_cache.view = m_cubeMapViews[face];
        m_perFrameBuffer.Update(m_deviceContext);
        m_perFrameBuffer.VSSet(m_deviceContext, 1);

        unsigned int viewportSize = Renderer::specularMapRes;
        for (int mipSlice = 0; mipSlice < Renderer::specularMapMipLevels; ++mipSlice, viewportSize = viewportSize >> 1) {
            // set viewport
            setViewport(viewportSize);
            float roughness = float(mipSlice) / float(Renderer::specularMapMipLevels - 1.0f);
            m_specularRoughnessBuffer.m_cache.fourFloats.x = roughness;
            m_specularRoughnessBuffer.Update(m_deviceContext);
            m_specularRoughnessBuffer.PSSet(m_deviceContext, 0);

            // temporary render target view
            ComPtr<ID3D11RenderTargetView> rtv;
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};
            rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = mipSlice;
            rtvDesc.Texture2DArray.ArraySize = 1;
            rtvDesc.Texture2DArray.FirstArraySlice = face;
            D3D_THROW_IF_FAILED(m_device->CreateRenderTargetView(m_specularMap.cubeBuffer.Get(), &rtvDesc, rtv.GetAddressOf()),
                                "Failed to create render target view");

            m_deviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);
            m_deviceContext->ClearRenderTargetView(rtv.Get(), clearColor);

            renderCube();
        }
    }

    // unbound srv and rtv
    ID3D11ShaderResourceView* nullSrv = nullptr;
    ID3D11RenderTargetView* nullRtv = nullptr;
    m_deviceContext->PSSetShaderResources(0, 1, &nullSrv);
    m_deviceContext->OMSetRenderTargets(1, &nullRtv, nullptr);
}

void D3d11RendererImpl::calculateCubemapMatrices() {
    CubeCamera cubeCamera(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_cubeMapPerspective = cubeCamera.ProjectionMatrixD3d();
    cubeCamera.ViewMatricesD3d(m_cubeMapViews);
}

void D3d11RendererImpl::cleanupImmediateRenderTarget() {
    if (m_immediate.rtv != nullptr)
        m_immediate.rtv->Release();
    if (m_immediate.dsv != nullptr)
        m_immediate.dsv->Release();
}

void D3d11RendererImpl::compileShaders() {
    m_pbrProgram.create(m_device, "PBR Program", "pbr");
    m_pbrModelProgram.create(m_device, "PBR Model Program", "pbr_model");
    m_convertProgram.create(m_device, "Convert Program", "cubemap", "to_cubemap");
    m_irradianceProgram.create(m_device, "Irradiance Program", "cubemap", "irradiance");
    m_backgroundProgram.create(m_device, "Background Program", "background");
    m_prefilterProgram.create(m_device, "Prefilter Program", "cubemap", "prefilter");
}

void D3d11RendererImpl::createGeometries() {
    // model
    const auto model = utility::LoadModel(g_model_dir.c_str());
    {
        // vertex buffer
        D3D11_BUFFER_DESC bufferDesc {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(TexturedVertex) * model.vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data {};
        data.pSysMem = model.vertices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_model.vertexBuffer.GetAddressOf()),
                            "Failed to create vertex buffer");
    }
    {
        // index buffer
        m_model.indexCount = static_cast<uint32_t>(3 * model.indices.size());
        D3D11_BUFFER_DESC bufferDesc {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(uvec3) * model.indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data {};
        data.pSysMem = model.indices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_model.indexBuffer.GetAddressOf()),
                            "Failed to create index buffer");
    }
    {
        // input layout
        D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TexturedVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(TexturedVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TexturedVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TexturedVertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TexturedVertex, bitangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HRESULT hr = m_device->CreateInputLayout(
            inputElementDescs,
            ARRAYSIZE(inputElementDescs),
            m_pbrModelProgram.vertShaderBlob->GetBufferPointer(),
            m_pbrModelProgram.vertShaderBlob->GetBufferSize(),
            m_texturedMeshLayout.GetAddressOf());

        D3D_THROW_IF_FAILED(hr, "Failed to model sphere input layout");
    }
    // sphere
    const auto sphere = CreateSphereMesh();
    {
        // vertex buffer
        D3D11_BUFFER_DESC bufferDesc {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(Vertex) * sphere.vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data {};
        data.pSysMem = sphere.vertices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_sphere.vertexBuffer.GetAddressOf()),
                            "Failed to create vertex buffer");
    }
    {
        // index buffer
        m_sphere.indexCount = static_cast<uint32_t>(3 * sphere.indices.size());
        D3D11_BUFFER_DESC bufferDesc {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = static_cast<uint32_t>(sizeof(uvec3) * sphere.indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA data {};
        data.pSysMem = sphere.indices.data();
        D3D_THROW_IF_FAILED(m_device->CreateBuffer(&bufferDesc, &data, m_sphere.indexBuffer.GetAddressOf()),
                            "Failed to create index buffer");
    }
    {
        // input layout
        D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HRESULT hr = m_device->CreateInputLayout(
            inputElementDescs,
            ARRAYSIZE(inputElementDescs),
            m_pbrProgram.vertShaderBlob->GetBufferPointer(),
            m_pbrProgram.vertShaderBlob->GetBufferSize(),
            m_meshLayout.GetAddressOf());

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
        D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HRESULT hr = m_device->CreateInputLayout(
            inputElementDescs,
            ARRAYSIZE(inputElementDescs),
            m_convertProgram.vertShaderBlob->GetBufferPointer(),
            m_convertProgram.vertShaderBlob->GetBufferSize(),
            m_cubeLayout.GetAddressOf());

        D3D_THROW_IF_FAILED(hr, "Failed to create cube input layout");
    }
}

void D3d11RendererImpl::createTexture2D(ComPtr<ID3D11ShaderResourceView>& srv, const Image& image, DXGI_FORMAT format) {
    D3D11_TEXTURE2D_DESC textureDesc {};
    textureDesc.Width = image.width;
    textureDesc.Height = image.height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA textureData {};
    textureData.pSysMem = image.buffer.pData;
    textureData.SysMemPitch = image.width * image.component;
    if (image.dataType == DataType::FLOAT_32T)
        textureData.SysMemPitch *= sizeof(float);
    textureData.SysMemSlicePitch = image.height * textureData.SysMemPitch;

    ComPtr<ID3D11Texture2D> texture;
    D3D_THROW_IF_FAILED(m_device->CreateTexture2D(&textureDesc, &textureData, texture.GetAddressOf()),
                        "Failed to create texture");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture1D.MipLevels = -1;

    D3D_THROW_IF_FAILED(m_device->CreateShaderResourceView(texture.Get(), &srvDesc, srv.GetAddressOf()),
                        "Failed to create shader resource view");
}

}  // namespace d3d11
}  // namespace pbr
