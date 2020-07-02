#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <d3d11.h>

namespace pbr { namespace d3d11 {
    using Microsoft::WRL::ComPtr;
} } // namespace pbr::d3d11

#define HLSL_DIR DATA_DIR "shaders/hlsl/"
#define DEFAULT_HDR_ENV_MAP DATA_DIR "preload/circus.hdr"
