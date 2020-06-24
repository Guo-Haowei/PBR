#include "VkHelpers.h"
#include "VkDebug.h"
#include "Utility.h"

namespace pbr { namespace vk {

VkShaderModule CreateShaderModuleFromFile(const VkDevice& device, const char* file)
{
    auto byteCode = utility::ReadBinaryFile(file);
    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = byteCode.size();
    info.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());
    VkShaderModule module {};
    VK_THROW_IF_FAILED(vkCreateShaderModule(device, &info, nullptr, &module),
        "Failed to create shader module");

    return module;
}

} } // namespace pbr::vk
