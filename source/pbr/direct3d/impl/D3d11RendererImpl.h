#pragma once
#include "D3d11Helpers.h"
#include "D3dPrerequisites.h"
#include "core/Renderer.h"


namespace pbr {
namespace d3d11 {

class D3d11RendererImpl {
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
    void createCubemap(CubemapTexture& inCubemap, int res, int mipLevels = 1, bool genMips = false);
    void cleanupImmediateRenderTarget();
    void compileShaders();
    void createGeometries();
    void setViewport(int width, int height = -1);
    void renderToEnvironmentMap();
    void renderToIrradianceMap();
    void renderToSpecularMap();
    void renderCube();
    void renderSpheres();
    void renderModel();
    void calculateCubemapMatrices();
    void uploadConstantBuffer();
    void createSampler();
    void setSrvAndSamplers();
    void createTexture2D(ComPtr<ID3D11ShaderResourceView>& srv, const Image& image, DXGI_FORMAT format);

   private:
    const Window* m_pWindow;
    HWND m_hwnd;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<IDXGIDevice> m_dxgiDevice;
    ComPtr<IDXGIAdapter> m_dxgiAdapter;
    ComPtr<IDXGIFactory> m_dxgiFactory;
    ComPtr<IDXGISwapChain> m_swapChain;
    // render target
    ImmediateRenderTarget m_immediate;
    // shaders
    HlslProgram m_pbrProgram;
    HlslProgram m_pbrModelProgram;
    HlslProgram m_convertProgram;
    HlslProgram m_irradianceProgram;
    HlslProgram m_prefilterProgram;
    HlslProgram m_backgroundProgram;
    // input layout
    ComPtr<ID3D11InputLayout> m_cubeLayout;
    ComPtr<ID3D11InputLayout> m_meshLayout;
    ComPtr<ID3D11InputLayout> m_texturedMeshLayout;
    // buffers
    ComPtr<ID3D11Buffer> m_constantBuffer;
    PerFrameBuffer m_perFrameBuffer;
    PerDrawBuffer m_perDrawBuffer;
    LightBuffer m_lightBuffer;
    ViewPositionBuffer m_viewPositionBuffer;
    PerDrawData m_cube;
    PerDrawData m_sphere;
    PerDrawData m_model;
    // rasterizer
    ComPtr<ID3D11RasterizerState> m_rasterizer;
    // reverse depth
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    // textures
    ComPtr<ID3D11ShaderResourceView> m_hdrSrv;
    ComPtr<ID3D11ShaderResourceView> m_brdfLUTSrv;
    ComPtr<ID3D11ShaderResourceView> m_albedoMetallic;
    ComPtr<ID3D11ShaderResourceView> m_normalRoughness;
    ComPtr<ID3D11ShaderResourceView> m_emissiveAO;
    CubemapTexture m_environmentMap;
    CubemapTexture m_irradianceMap;
    CubemapTexture m_specularMap;
    // sampler
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11SamplerState> m_samplerLod;
    // matrices
    mat4 m_cubeMapPerspective;
    array<mat4, 6> m_cubeMapViews;
};

}  // namespace d3d11
}  // namespace pbr
