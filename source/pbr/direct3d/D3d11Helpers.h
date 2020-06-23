#pragma once
#include "base/Definitions.h"
#include "D3dDebug.h"

namespace pbr { namespace d3d11 {

struct PerFrameCache
{
    mat4 view;
    mat4 projection;
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

static mat4& convertProjection(mat4& projection)
{
    // projection = mat4( { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0.5, 1} ) *
    //              mat4( { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0.5, 0 }, { 0, 0, 0, 1} ) *
    //              projection;
    return projection;
}

template<class Cache> class ConstantBuffer
{
public:
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

class HlslShader
{
public:
    static void CompileShader(const char* file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob>& sourceBlob);
};

} } // namespace pbr::d3d11
