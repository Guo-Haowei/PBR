#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

namespace pbr { namespace d3d11 {
    using Microsoft::WRL::ComPtr;
} } // namespace pbr::d3d11
