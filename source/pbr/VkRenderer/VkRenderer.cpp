#include "VkRenderer.h"
#include "VkDebug.h"
#include "Window.h"
#include <set>

namespace pbr {
    using std::set;
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
}

void VkRenderer::DumpGraphicsCardInfo()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
    std::cout << "Graphics Card:     " << deviceProperties.deviceName << std::endl;
}

void VkRenderer::Render()
{

}

void VkRenderer::Resize(const Extent2i& extent)
{

}

void VkRenderer::Finalize()
{
    vkDestroyDevice(m_device, m_allocator);
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
    info.enabledExtensionCount = extensions.size();
    info.ppEnabledExtensionNames = extensions.data();
#ifdef PBR_DEBUG
    info.enabledLayerCount = g_requestedLayers.size();
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
// #ifdef VK_VERBOSE
//     std::cout << "[Info] Vulkan: " << devices.size() << " available physical devices\n";
// #endif
//     for (const auto& device : devices)
//     {
//         VkPhysicalDeviceProperties deviceProperties;
//         VkPhysicalDeviceFeatures deviceFeatures;
//         vkGetPhysicalDeviceProperties(device, &deviceProperties);
//         vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

// #ifdef VK_VERBOSE
//         std::cout << "\t" << deviceProperties.deviceName << "\n";
// #endif
//     }

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
    deviceInfo.queueCreateInfoCount = deviceQueueInfos.size();
    deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
    deviceInfo.pEnabledFeatures = &features;
#ifdef PBR_DEBUG
    deviceInfo.enabledLayerCount = g_requestedLayers.size();
    deviceInfo.ppEnabledLayerNames = g_requestedLayers.data();
#endif

    VK_THROW_IF_FAILED(vkCreateDevice(m_physicalDevice, &deviceInfo, m_allocator, &m_device),
        "Failed to create logical device");

    vkGetDeviceQueue(m_device, m_queueFamily.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamily.presentFamily.value(), 0, &m_presentQueue);
}

} // namespace pbr
