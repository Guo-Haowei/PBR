#pragma once
#include "VkPrerequisites.h"

namespace pbr { namespace vk {

extern VkShaderModule createShaderModuleFromFile(const char* file, const VkDevice& device, VkAllocationCallbacks* pAlloc);

struct PipelineViewportState
{
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo createInfo;

    PipelineViewportState(int width, int height)
    {
        setExtent(width, height);
    }

    void setExtent(int width, int height)
    {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = width;
        scissor.extent.height = height;

        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.viewportCount = 1;
        createInfo.pViewports = &viewport;
        createInfo.scissorCount = 1;
        createInfo.pScissors = &scissor;
    }
};

} } // namespace pbr::vk
