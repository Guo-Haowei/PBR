#include "core/Window.h"
#include "VkRenderer.h"
#include "VkDebug.h"
#include "VkHelpers.h"
#include "Utility.h"
#include <array>
#include <set>

namespace pbr {
    using std::set;
    using std::array;
} // namespace pbr

namespace pbr {

const vector<const char*> g_requestedLayers = {
#if TARGET_PLATFORM == PLATFORM_WINDOWS
    "VK_LAYER_KHRONOS_validation"
#endif
};

VkRenderer::VkRenderer(const Window* pWindow) : Renderer(pWindow)
{
}

void VkRenderer::Initialize()
{
    checkValidationLayers(g_requestedLayers);
    createVkInstance();
    setDebugCallback();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createSwapChainImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createSwapChainFramebuffers();
    createCommandPool();
    createCommandBuffers();
}

void VkRenderer::DumpGraphicsCardInfo()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
    std::cout << "Graphics Card:     " << deviceProperties.deviceName << std::endl;
}

void VkRenderer::PrepareGpuResources()
{
}

void VkRenderer::Render()
{
    // VkPresentInfoKHR presentInfo {};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = &m_swapChain;
    // vkQueuePresentKHR(m_presentQueue, &presentInfo);
}

void VkRenderer::Resize(const Extent2i& extent)
{

}

void VkRenderer::Finalize()
{
    // command pool
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator);
    // framebuffers
    for (auto& framebuffer : m_swapChainFramebuffers)
        vkDestroyFramebuffer(m_logicalDevice, framebuffer, m_allocator);
    // pipeline
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, m_allocator);
    // pipeline layout
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, m_allocator);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, m_allocator);
    for (auto& imageView : m_swapChainImageViews)
        vkDestroyImageView(m_logicalDevice, imageView, m_allocator);
    vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, m_allocator);
    vkDestroyDevice(m_logicalDevice, m_allocator);
#ifdef PBR_DEBUG
    auto vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
    assert(vkDestroyDebugReportCallbackEXT != NULL);
    vkDestroyDebugReportCallbackEXT(m_instance, m_debugHandle, m_allocator);
#endif
    vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
    vkDestroyInstance(m_instance, m_allocator);
}

void VkRenderer::createVkInstance()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef PBR_DEBUG
    extensions.push_back("VK_EXT_debug_report");
#endif

    VkInstanceCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
#ifdef PBR_DEBUG
    info.enabledLayerCount = static_cast<uint32_t>(g_requestedLayers.size());
    info.ppEnabledLayerNames = g_requestedLayers.data();
#endif
    VK_THROW_IF_FAILED(vkCreateInstance(&info, m_allocator, &m_instance), "Failed to create instance");
}

void VkRenderer::setDebugCallback()
{
#ifdef PBR_DEBUG
    auto vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
    assert(vkCreateDebugReportCallbackEXT != NULL);

    VkDebugReportCallbackCreateInfoEXT info {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    info.pfnCallback = DebugReport;
    info.pUserData = NULL;
    VK_THROW_IF_FAILED(vkCreateDebugReportCallbackEXT(m_instance, &info, m_allocator, &m_debugHandle),
        "Failed to find creaet debug report callback");
#endif
}

void VkRenderer::createSurface()
{
    VK_THROW_IF_FAILED(glfwCreateWindowSurface(m_instance, m_pWindow->GetInternalWindow(), m_allocator, &m_surface),
        "Failed to create window surface");
}

void VkRenderer::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        THROW_EXCEPTION("Vulkan: Failed to obtain any physical devices");
    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    // simply choose the first gpu we found
    m_physicalDevice = devices.front();

    // queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0)
        THROW_EXCEPTION("Vulkan: Failed to obtain any queue family");
    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& property = queueFamilies[i];
        if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_queueFamily.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport)
            m_queueFamily.presentFamily = i;

        if (m_queueFamily.has_value())
            break;
    }

    assert(m_queueFamily.has_value());
}

void VkRenderer::createLogicalDevice()
{
    // extension
    vector<const char*> deviceExtensions = { "VK_KHR_swapchain" };
    float queuePriority = 1.0f;

    set<uint32_t> queueFamilySet = { m_queueFamily.graphicsFamily.value(), m_queueFamily.presentFamily.value() };
    vector<VkDeviceQueueCreateInfo> deviceQueueInfos;
    for (uint32_t index : queueFamilySet)
    {
        VkDeviceQueueCreateInfo deviceQueueInfo {};
        deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueInfo.queueCount = 1;
        deviceQueueInfo.queueFamilyIndex = index;
        deviceQueueInfo.pQueuePriorities = &queuePriority;
        deviceQueueInfos.push_back(deviceQueueInfo);
    }

    VkPhysicalDeviceFeatures features {};
    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfos.size());
    deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
    deviceInfo.pEnabledFeatures = &features;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
#ifdef PBR_DEBUG
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(g_requestedLayers.size());
    deviceInfo.ppEnabledLayerNames = g_requestedLayers.data();
#endif

    VK_THROW_IF_FAILED(vkCreateDevice(m_physicalDevice, &deviceInfo, m_allocator, &m_logicalDevice),
        "Failed to create logical device");

    vkGetDeviceQueue(m_logicalDevice, m_queueFamily.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, m_queueFamily.presentFamily.value(), 0, &m_presentQueue);
}

void VkRenderer::createSwapChain()
{
    // query support
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);

    uint32_t count;
    // surface format
    count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &count, nullptr);
    if (count == 0)
        THROW_EXCEPTION("Vulkan: Failed to obtain any surface formats");
    vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &count, formats.data());
    // present mode
    count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &count, nullptr);
    if (count == 0)
        THROW_EXCEPTION("Vulkan: Failed to obtain any present modes");
    vector<VkPresentModeKHR> presentModes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &count, presentModes.data());

    // choose format
    VkSurfaceFormatKHR surfaceFormat = formats.front();
    for (const auto& available : formats)
    {
        if (available.format == VK_FORMAT_B8G8R8A8_SRGB && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = available;
            break;
        }
    }

    // choose present mode
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& available : presentModes)
    {
        if (available == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            swapchainPresentMode = available;
            break;
        }
    }

    // swapchain extent
    VkExtent2D extent;
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        extent = surfaceCapabilities.currentExtent;
    }
    else
    {
        const Extent2i& windowExtent = m_pWindow->GetFrameBufferExtent();
        extent.width = glm::clamp((uint32_t)windowExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = glm::clamp((uint32_t)windowExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    // image count
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
        imageCount = surfaceCapabilities.maxImageCount;

    // create
    VkSwapchainCreateInfoKHR info {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = m_surface;
    info.minImageCount = imageCount;
    info.imageFormat = surfaceFormat.format;
    info.imageColorSpace = surfaceFormat.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    array<uint32_t, 2> queueFamilies = { m_queueFamily.graphicsFamily.value(), m_queueFamily.presentFamily.value() };
    if (queueFamilies[0] != queueFamilies[1])
    {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
        info.pQueueFamilyIndices = queueFamilies.data();
    }
    else
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // TODO: configure NDC
    info.preTransform = surfaceCapabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = swapchainPresentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = VK_NULL_HANDLE;

    VK_THROW_IF_FAILED(vkCreateSwapchainKHR(m_logicalDevice, &info, m_allocator, &m_swapChain),
        "Failed to create swap chain");

    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void VkRenderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = m_swapChainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VK_THROW_IF_FAILED(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, m_allocator, &m_renderPass),
        "Failed to create render pass");
}

void VkRenderer::createSwapChainImageViews()
{
    // swapchain image views
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        VkImageViewCreateInfo imageViewInfo {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = m_swapChainImages[i];
        imageViewInfo.format = m_swapChainFormat;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        VK_THROW_IF_FAILED(vkCreateImageView(m_logicalDevice, &imageViewInfo, m_allocator, &m_swapChainImageViews[i]),
            "Failed to create swap chain image view");
    }
}

void VkRenderer::createGraphicsPipeline()
{
    // shader
    VkShaderModule vertexShaderModule = vk::createShaderModuleFromFile(SPIRV_DIR "pbr.vert.spirv", m_logicalDevice, m_allocator);
    VkShaderModule fragmentShaderModule = vk::createShaderModuleFromFile(SPIRV_DIR "pbr.frag.spirv", m_logicalDevice, m_allocator);

    VkPipelineShaderStageCreateInfo vertCreateInfo {};
    vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.module = vertexShaderModule;
    vertCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragCreateInfo {};
    fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragCreateInfo.module = fragmentShaderModule;
    fragCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertCreateInfo, fragCreateInfo };

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // viewport
    vk::PipelineViewportState viewport(m_swapChainExtent.width, m_swapChainExtent.height);

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;

    // color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT ;
    colorBlendAttachment.blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;

    // pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.setLayoutCount = 0;
    // pipelineLayoutInfo.pushConstantRangeCount = 0;

    VK_THROW_IF_FAILED(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, m_allocator, &m_pipelineLayout),
        "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // shader stages
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    // vertex input skip for now

    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewport.createInfo;

    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlendInfo;

    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_THROW_IF_FAILED(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, m_allocator, &m_graphicsPipeline),
        "Failed to create graphics pipeline");

    // destroy shader module
    vkDestroyShaderModule(m_logicalDevice, vertexShaderModule, m_allocator);
    vkDestroyShaderModule(m_logicalDevice, fragmentShaderModule, m_allocator);
}

void VkRenderer::createSwapChainFramebuffers()
{
    // swapchain framebuffers
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
    for (uint32_t i = 0; i < m_swapChainFramebuffers.size(); ++i)
    {
        VkImageView attachments[] = { m_swapChainImageViews[i] };
        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        VK_THROW_IF_FAILED(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, m_allocator, &m_swapChainFramebuffers[i]),
            "Failed to create frame buffer " + std::to_string(i));
    }
}

void VkRenderer::createCommandPool()
{
    VkCommandPoolCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = m_queueFamily.graphicsFamily.value();
    info.flags = 0;

    VK_THROW_IF_FAILED(vkCreateCommandPool(m_logicalDevice, &info, m_allocator, &m_commandPool),
        "Failed to create command pool");
}

void VkRenderer::createCommandBuffers()
{
    m_commandBuffers.resize(m_swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    VK_THROW_IF_FAILED(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data()),
        "Failed to create command buffers");

    for (size_t i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_THROW_IF_FAILED(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo),
            "Failed to begin recording command buffer");

        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChainExtent;

        VkClearValue clearColor = { 0.3f, 0.3f, 0.4f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
        //vkCmdDraw(m_commandBuffers[i], 2, 1, 0, 0);
        vkCmdEndRenderPass(m_commandBuffers[i]);

        VK_THROW_IF_FAILED(vkEndCommandBuffer(m_commandBuffers[i]),
            "Failed to record command buffer");
    }
}

} // namespace pbr
