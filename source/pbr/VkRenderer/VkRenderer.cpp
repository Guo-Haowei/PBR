#include "VkRenderer.h"
#include "Window.h"
#include "Error.h"

namespace pbr {

VkRenderer::VkRenderer(const Window* pWindow) : Renderer(pWindow)
{
}

void VkRenderer::Initialize()
{
    createVkInstance();
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

}

void VkRenderer::createVkInstance()
{
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.enabledExtensionCount = glfwExtensionCount;
    info.ppEnabledExtensionNames = glfwExtensions;
    info.enabledLayerCount = 0;
    VK_THROW_IF_FAILED(vkCreateInstance(&info, nullptr, &m_instance), "Failed to create instance");
}

} // namespace pbr
