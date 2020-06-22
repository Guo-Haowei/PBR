#pragma once
#include "D3dPrerequisites.h"

namespace pbr {

class HlslShader
{
public:
    static void CompileShader(const char* file, LPCSTR entry, LPCSTR target, ComPtr<ID3DBlob>& sourceBlob);
};

} // namespace pbr
