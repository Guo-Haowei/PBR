#include "VkHelpers.h"
#include "VkDebug.h"
#include "Utility.h"

namespace pbr { namespace vk {

VkShaderModule createShaderModuleFromFile(const char* file, const VkDevice& device, VkAllocationCallbacks* pAlloc)
{
    auto byteCode = utility::readBinaryFile(file);
    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = byteCode.size();
    info.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());
    VkShaderModule module {};
    VK_THROW_IF_FAILED(vkCreateShaderModule(device, &info, pAlloc, &module),
        "Failed to create shader module");

    return module;
}

} } // namespace pbr::vk
