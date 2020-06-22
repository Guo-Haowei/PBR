#pragma once
#include "D3dPrerequisites.h"
#include "base/Error.h"
#include <comdef.h>

namespace pbr {

    class D3dException : public Exception
    {
    public:
        D3dException(int line, const char* file, const char* desc, HRESULT hr)
            : Exception(line, file, desc), m_result(hr)
        {
        }

        D3dException(int line, const char* file, const string& desc, HRESULT hr)
            : Exception(line, file, desc), m_result(hr)
        {
        }

        virtual ostream& dump(ostream& os) const
        {
            os << "[Error] Direct3d: " << m_desc << ".\n\ton line " << m_line << ", in file [" << m_file << "]";
            _com_error error(m_result);
            os << "\n\treason: " << error.ErrorMessage();
            return os;
        }
    protected:
        HRESULT m_result;
    };
} // namespace pbr

#define D3D_THROW_IF_FAILED(EXP, DESC) \
{ \
    HRESULT _HR = (EXP); \
    if (_HR != S_OK) throw pbr::D3dException(__LINE__, __FILE__, DESC, _HR); \
}
