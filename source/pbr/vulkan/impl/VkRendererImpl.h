#pragma once
#include "VkPrerequisites.h"
#include "VkHelpers.h"
#include "core/Camera.h"
#include "core/Window.h"

namespace pbr { namespace vk {

class VkRendererImpl
{
public:
    VkRendererImpl(const Window* pWindow);
    void Initialize();
    void DumpGraphicsCardInfo();
    void PrepareGpuResources();
    void Render(const Camera& camera);
    void Resize(const Extent2i& extent);
    void Finalize();
private:
    void createVkInstance();
    void setDebugCallback();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createSwapChainImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createSwapChainFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();
private:
    const Window*               m_pWindow               = nullptr;
    VkInstance                  m_instance              = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT    m_debugHandle           = VK_NULL_HANDLE;
    VkPhysicalDevice            m_physicalDevice        = VK_NULL_HANDLE;
    QueueFamilyIndices          m_queueFamily;
    VkDevice                    m_logicalDevice         = VK_NULL_HANDLE;
    VkQueue                     m_graphicsQueue         = VK_NULL_HANDLE;
    VkQueue                     m_presentQueue          = VK_NULL_HANDLE;
    VkSurfaceKHR                m_surface               = VK_NULL_HANDLE;
    // swap chain
    VkSwapchainKHR              m_swapChain             = VK_NULL_HANDLE;
    vector<VkImage>             m_swapChainImages;
    VkFormat                    m_swapChainFormat;
    VkExtent2D                  m_swapChainExtent;
    vector<VkImageView>         m_swapChainImageViews;
    vector<VkFramebuffer>       m_swapChainFramebuffers;
    // render pass
    VkRenderPass                m_renderPass            = VK_NULL_HANDLE;
    VkPipelineLayout            m_pipelineLayout        = VK_NULL_HANDLE;
    VkPipeline                  m_graphicsPipeline      = VK_NULL_HANDLE;
    // command buffer
    VkCommandPool               m_commandPool           = VK_NULL_HANDLE;
    vector<VkCommandBuffer>     m_commandBuffers;
    VkSemaphore                 m_imageAvailableMutex   = VK_NULL_HANDLE;
    VkSemaphore                 m_renderFinishedMutex   = VK_NULL_HANDLE;
};

} } // namespace pbr::vk