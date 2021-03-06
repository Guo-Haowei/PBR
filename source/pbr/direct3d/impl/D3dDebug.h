#pragma once
#include <comdef.h>
#include "D3dPrerequisites.h"
#include "base/Error.h"

namespace pbr {
namespace d3d11 {

class D3dException : public Exception {
   public:
    D3dException(int line, const char* file, const char* desc, HRESULT hr)
        : Exception(line, file, desc), m_result(hr) {
    }

    D3dException(int line, const char* file, const string& desc, HRESULT hr)
        : Exception(line, file, desc), m_result(hr) {
    }

    virtual ostream& dump(ostream& os) const {
        os << "[Error] Direct3d: " << m_desc << ".\n\ton line " << m_line << ", in file [" << m_file << "]";
        _com_error error(m_result);
        os << "\n\treason: " << error.ErrorMessage();
        return os;
    }

   protected:
    HRESULT m_result;
};
}  // namespace d3d11
}  // namespace pbr

#ifdef _DEBUG
#define D3D_THROW_IF_FAILED(EXP, DESC)   \
    {                                    \
        HRESULT _HR = (EXP);             \
        if (_HR != S_OK) __debugbreak(); \
    }
#else
#define D3D_THROW_IF_FAILED(EXP, DESC)                                                  \
    {                                                                                   \
        HRESULT _HR = (EXP);                                                            \
        if (_HR != S_OK) throw pbr::d3d11::D3dException(__LINE__, __FILE__, DESC, _HR); \
    }
#endif
