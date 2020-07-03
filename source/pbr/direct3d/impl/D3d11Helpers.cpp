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

} } // namespace pbr::d3d11
