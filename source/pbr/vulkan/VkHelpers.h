#pragma once
#include "VkPrerequisites.h"

namespace pbr {

extern VkShaderModule createShaderModuleFromFile(const char* file, const VkDevice& device, VkAllocationCallbacks* pAlloc);

} // namespace pbr
