#include "D3d11Helpers.h"
#include "D3dDebug.h"
#include <d3dcompiler.h>

namespace pbr { namespace d3d11 {

    void HlslProgram::CompileShader(string const& file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob>& sourceBlob)
    {
        string fileStr(file);
        wstring fileWStr(fileStr.begin(), fileStr.end());
        ComPtr<ID3DBlob> errorBlob;
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef PBR_DEBUG
        flags |= D3DCOMPILE_DEBUG;
#endif
        HRESULT hr = D3DCompileFromFile(
            fileWStr.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entry,
            target,
            flags,
            0,
            sourceBlob.GetAddressOf(),
            errorBlob.GetAddressOf());

        if (FAILED(hr))
        {
            if (errorBlob != nullptr)
            {
                const char* errorCStr = (const char*)errorBlob->GetBufferPointer();
                string error("hlsl: Failed to compile shader\n");
                error.append(errorCStr).pop_back();
                THROW_EXCEPTION(error);
            }
            else
            {
                D3D_THROW_IF_FAILED(hr, "Failed to compile shader");
            }
        }
    }

    void HlslProgram::create(ComPtr<ID3D11Device>& device, const char* debugName, char const* vertName, const char* fragName)
    {
        string path(HLSL_DIR);
        string vertFile(vertName); vertFile.append(".vert.hlsl");
        string pixelFile(fragName == nullptr ? vertName : fragName); pixelFile.append(".pixel.hlsl");
        SHADER_COMPILING_START_INFO(debugName);
        HlslProgram::CompileShader(path + vertFile, "vs_main", "vs_5_0", vertShaderBlob);
        HRESULT hr = device->CreateVertexShader(
            vertShaderBlob->GetBufferPointer(),
            vertShaderBlob->GetBufferSize(),
            NULL,
            vertShader.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create vertex shader");
        ComPtr<ID3DBlob> pixelBlob;
        HlslProgram::CompileShader(path + pixelFile, "ps_main", "ps_5_0", pixelBlob);
        hr = device->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            NULL,
            pixelShader.GetAddressOf());
        D3D_THROW_IF_FAILED(hr, "Failed to create pixel shader");
        SHADER_COMPILING_END_INFO(debugName);
    }

    void HlslProgram::set(ComPtr<ID3D11DeviceContext>& deviceContext)
    {
        deviceContext->VSSetShader(vertShader.Get(), 0, 0);
        deviceContext->PSSetShader(pixelShader.Get(), 0, 0);
    }

    Texture2D* CreateHDRTexture(ComPtr<ID3D11Device>& device, const Image& image)
    {
        DXGI_FORMAT format;
        switch (image.component)
        {
            case 4: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            case 3: format = DXGI_FORMAT_R32G32B32_FLOAT; break;
            default:
                THROW_EXCEPTION("[texture] Unsupported image format, image has component " + std::to_string(image.component));
        }

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
        textureData.SysMemPitch = image.width * image.component * sizeof(float);
        textureData.SysMemSlicePitch = image.height * textureData.SysMemPitch;

        ComPtr<ID3D11Texture2D> texture;
        D3D_THROW_IF_FAILED(device->CreateTexture2D(&textureDesc, &textureData, texture.GetAddressOf()),
            "Failed to create hdr texture");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture1D.MipLevels = -1;

        ID3D11ShaderResourceView* pSrv;
        D3D_THROW_IF_FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &pSrv),
            "Failed to create shader resource view");

        D3D11_SAMPLER_DESC samplerDesc {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        ID3D11SamplerState* pSamplerState;
        D3D_THROW_IF_FAILED(device->CreateSamplerState(&samplerDesc, &pSamplerState),
            "Failed to create sampler satete");

        return new Texture2D(pSrv, pSamplerState);
    }

} } // namespace pbr::d3d11
