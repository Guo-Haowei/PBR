#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <stdexcept>
#include <comdef.h>

namespace pbr {
    using Microsoft::WRL::ComPtr;
    using std::runtime_error;
    using std::string;
} // namespace pbr

#define THROW_IF_NOT_OK(_EXPRESSION, _ERROR) \
{ \
    HRESULT _HR = (_EXPRESSION); \
    if (_HR != S_OK) \
    { \
        std::string _FULL_ERROR(_ERROR); \
        _com_error _COM_ERROR(_HR); \
        _FULL_ERROR.append("\n\treason: ").append(std::string(_COM_ERROR.ErrorMessage())); \
        throw runtime_error(_FULL_ERROR); \
    } \
}
