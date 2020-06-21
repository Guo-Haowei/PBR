#pragma once
#include "VkPrerequisites.h"
#include "Error.h"
#include <iostream>
#include <unordered_map>

namespace pbr {

    static const string& VkResultToString(VkResult result)
    {
#define VKRESULT_ENTRY(E) { E, #E }
        static const std::unordered_map<int, const string> sTable = {
            VKRESULT_ENTRY(VK_SUCCESS),
            VKRESULT_ENTRY(VK_NOT_READY),
            VKRESULT_ENTRY(VK_TIMEOUT),
            VKRESULT_ENTRY(VK_EVENT_SET),
            VKRESULT_ENTRY(VK_EVENT_RESET),
            VKRESULT_ENTRY(VK_INCOMPLETE),
            VKRESULT_ENTRY(VK_ERROR_OUT_OF_HOST_MEMORY),
            VKRESULT_ENTRY(VK_ERROR_OUT_OF_DEVICE_MEMORY),
            VKRESULT_ENTRY(VK_ERROR_INITIALIZATION_FAILED),
            VKRESULT_ENTRY(VK_ERROR_DEVICE_LOST),
            VKRESULT_ENTRY(VK_ERROR_MEMORY_MAP_FAILED),
            VKRESULT_ENTRY(VK_ERROR_LAYER_NOT_PRESENT),
            VKRESULT_ENTRY(VK_ERROR_EXTENSION_NOT_PRESENT),
            VKRESULT_ENTRY(VK_ERROR_FEATURE_NOT_PRESENT),
            VKRESULT_ENTRY(VK_ERROR_INCOMPATIBLE_DRIVER),
            VKRESULT_ENTRY(VK_ERROR_TOO_MANY_OBJECTS),
            VKRESULT_ENTRY(VK_ERROR_FORMAT_NOT_SUPPORTED),
            VKRESULT_ENTRY(VK_ERROR_FRAGMENTED_POOL),
//            VKRESULT_ENTRY(VK_ERROR_UNKNOWN),
            VKRESULT_ENTRY(VK_ERROR_OUT_OF_POOL_MEMORY),
            VKRESULT_ENTRY(VK_ERROR_INVALID_EXTERNAL_HANDLE),
//            VKRESULT_ENTRY(VK_ERROR_FRAGMENTATION),
//            VKRESULT_ENTRY(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS),
            VKRESULT_ENTRY(VK_ERROR_SURFACE_LOST_KHR),
            VKRESULT_ENTRY(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR),
            VKRESULT_ENTRY(VK_SUBOPTIMAL_KHR),
            VKRESULT_ENTRY(VK_ERROR_OUT_OF_DATE_KHR),
            VKRESULT_ENTRY(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR),
            VKRESULT_ENTRY(VK_ERROR_VALIDATION_FAILED_EXT),
            VKRESULT_ENTRY(VK_ERROR_INVALID_SHADER_NV),
//            VKRESULT_ENTRY(VK_ERROR_INCOMPATIBLE_VERSION_KHR),
            VKRESULT_ENTRY(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT),
            VKRESULT_ENTRY(VK_ERROR_NOT_PERMITTED_EXT),
            VKRESULT_ENTRY(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT),
//            VKRESULT_ENTRY(VK_THREAD_IDLE_KHR),
//            VKRESULT_ENTRY(VK_THREAD_DONE_KHR),
//            VKRESULT_ENTRY(VK_OPERATION_DEFERRED_KHR),
//            VKRESULT_ENTRY(VK_OPERATION_NOT_DEFERRED_KHR),
//            VKRESULT_ENTRY(VK_PIPELINE_COMPILE_REQUIRED_EXT),
            VKRESULT_ENTRY(VK_ERROR_OUT_OF_POOL_MEMORY_KHR),
            VKRESULT_ENTRY(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR),
            VKRESULT_ENTRY(VK_ERROR_FRAGMENTATION_EXT),
            VKRESULT_ENTRY(VK_ERROR_INVALID_DEVICE_ADDRESS_EXT),
//            VKRESULT_ENTRY(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR),
//            VKRESULT_ENTRY(VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT),
            VKRESULT_ENTRY(VK_RESULT_MAX_ENUM)};
#undef VKRESULT_ENTRY
        static const string sUnknown = "Unknown";
        auto found = sTable.find(result);
        return found == sTable.end() ? sUnknown : found->second;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
        std::cout <<
        "[vulkan] Debug report from ObjectType: " << objectType <<
        " \nMessage: " << pMessage << "\n\n";

        return VK_FALSE;
    }

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
            os << "\n\terror code: " << VkResultToString(m_result);
            return os;
        }
    protected:
        VkResult m_result;
    };
} // namespace pbr

#define VK_THROW(DESC, VKRESULT) throw pbr::VkException(__LINE__, __FILE__, DESC, VKRESULT)

#define VK_THROW_IF_FAILED(EXP, DESC) \
{ \
    VkResult _VR = (EXP); \
    if (_VR != VK_SUCCESS) throw pbr::VkException(__LINE__, __FILE__, DESC, _VR); \
}

