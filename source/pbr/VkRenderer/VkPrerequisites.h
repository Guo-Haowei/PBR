#pragma once
#include "Error.h"
#include "vulkan/vulkan.h"

namespace pbr {

    class VkException : public Exception
    {
    public:
        VkException(int line, const char* file, const char* desc, VkResult vr)
            : Exception(line, file, desc), m_result(vr)
        {
        }

        VkException(int line, const char* file, const string& desc, VkResult vr)
            : Exception(line, file, desc), m_result(vr)
        {
        }

        virtual ostream& dump(ostream& os) const
        {
            os << "[Error] Vulkan: " << m_desc << ".\n\ton line " << m_line << ", in file [" << m_file << "]";
            os << "\n\terror code: " << m_result;
            return os;
        }
    protected:
        VkResult m_result;
    };
} // namespace pbr

#define VK_THROW_IF_FAILED(EXP, DESC) \
{ \
    VkResult _VR = (EXP); \
    if (_VR != VK_SUCCESS) throw pbr::VkException(__LINE__, __FILE__, DESC, _VR); \
}

