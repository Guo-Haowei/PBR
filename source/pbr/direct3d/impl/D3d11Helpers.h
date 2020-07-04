#pragma once
#include "base/Definitions.h"
#include "D3dDebug.h"
#include "Scene.h"

namespace pbr { namespace d3d11 {

struct PerFrameCache
{
    mat4 view;
    mat4 projection;
};

struct Vec4Cache
{
    vec4 fourFloats;
};

struct PerDrawCache
{
    mat4 transform;
};

struct PerDrawData
{
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    uint32_t indexCount = 0;
};

struct LightDataCache
{
    array<Light, 4> lights;
};

struct ViewPositionCache
{
    vec3 view_position;
    int padding;
};

static_assert(sizeof(LightDataCache) == 4 * 2 * sizeof(vec4));

template<class Cache> class ConstantBuffer
{
public:
    inline size_t BufferSize() const { return sizeof(Cache); }

    ConstantBuffer() = default;

    void Create(ComPtr<ID3D11Device>& device)
    {
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = sizeof(Cache);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D_THROW_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, m_buffer.GetAddressOf()),
            "Failed to create constant buffer");
    }

    void Update(ComPtr<ID3D11DeviceContext>& deviceContext)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
        deviceContext->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, (void*)&m_cache, sizeof(Cache));
        deviceContext->Unmap(m_buffer.Get(), 0);
    }

    void VSSet(ComPtr<ID3D11DeviceContext>& deviceContext, uint32_t slot)
    {
        deviceContext->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
    }

    void PSSet(ComPtr<ID3D11DeviceContext>& deviceContext, uint32_t slot)
    {
        deviceContext->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
    }

public:
    Cache m_cache;
private:
    ComPtr<ID3D11Buffer> m_buffer;
};

typedef ConstantBuffer<PerFrameCache> PerFrameBuffer;
typedef ConstantBuffer<PerDrawCache> PerDrawBuffer;
typedef ConstantBuffer<LightDataCache> LightBuffer;
typedef ConstantBuffer<ViewPositionCache> ViewPositionBuffer;
typedef ConstantBuffer<Vec4Cache> FourFloatsBuffer;


struct HlslProgram
{
    static void CompileShader(string const& file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob>& sourceBlob);

    void create(ComPtr<ID3D11Device>& device, const char* debugName, char const* vertName, const char* fragName = nullptr);

    void set(ComPtr<ID3D11DeviceContext>& deviceContext);

    ComPtr<ID3D11VertexShader> vertShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3DBlob> vertShaderBlob;
};

struct ImmediateRenderTarget
{
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11DepthStencilView> dsv;
};

struct CubemapTexture
{
    ComPtr<ID3D11ShaderResourceView>            srv;
    ComPtr<ID3D11Texture2D>                     cubeBuffer;
};

} } // namespace pbr::d3d11
