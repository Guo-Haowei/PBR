#include "VkRenderer.h"
#include "VkDebug.h"
#include "Window.h"
#include <string.h>

namespace pbr {

VkRenderer::VkRenderer(const Window* pWindow) : Renderer(pWindow)
{
}

void VkRenderer::Initialize()
{
    createVkInstance();
    setDebugCallback();
}

void VkRenderer::DumpGraphicsCardInfo()
{

}

void VkRenderer::Render()
{

}

void VkRenderer::Resize(const Extent2i& extent)
{

}

void VkRenderer::Finalize()
{
#ifdef PBR_DEBUG
    auto vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
    assert(vkDestroyDebugReportCallbackEXT != NULL);
    vkDestroyDebugReportCallbackEXT(m_instance, m_debugHandle, nullptr);
#endif
    vkDestroyInstance(m_instance, nullptr);
}

void VkRenderer::createVkInstance()
{
    // check if layers exist
    const vector<string> availableLayers = getAvailalbeLayers();
    const vector<const char*> requestedLayers = {
#if TARGET_PLATFORM == PLATFORM_WINDOWS
        "VK_LAYER_KHRONOS_validation"
#endif
    };
    for (const char* requestedLayer : requestedLayers)
    {
        bool found = false;
        for (auto availableLayer : availableLayers)
            if (requestedLayer == availableLayer) found = true;

        if (!found)
            VK_THROW("Failed to find layer " + string(requestedLayer), VK_ERROR_LAYER_NOT_PRESENT);
    }

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
    info.enabledLayerCount = requestedLayers.size();
    info.ppEnabledLayerNames = requestedLayers.data();
#endif
    VK_THROW_IF_FAILED(vkCreateInstance(&info, nullptr, &m_instance), "Failed to create instance");
}

vector<string> VkRenderer::getAvailalbeLayers()
{
    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    vector<VkLayerProperties> layers(layerCount);
    vector<string> layerNames(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    for (const auto& layer : layers)
        layerNames.push_back(layer.layerName);
    return layerNames;
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
    VK_THROW_IF_FAILED(vkCreateDebugReportCallbackEXT(m_instance, &info, nullptr, &m_debugHandle),
        "Failed to find creaet debug report callback");
#endif
}

} // namespace pbr
