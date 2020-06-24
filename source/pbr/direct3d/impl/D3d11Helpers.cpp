#include "D3d11Helpers.h"
#include "D3dDebug.h"
#include <d3dcompiler.h>

namespace pbr { namespace d3d11 {

    void HlslShader::CompileShader(const char* file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob>& sourceBlob)
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

} } // namespace pbr::d3d11
