#pragma once
#include "Renderer.h"
#include "VkPrerequisites.h"
#include <optional>

namespace pbr {
    using std::optional;
} // namespace pbr

namespace pbr {

struct QueueFamilyIndices
{
    optional<uint32_t> graphicsFamily;
    optional<uint32_t> presentFamily;

    bool has_value() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class VkRenderer : public Renderer
{
public:
    VkRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void DumpGraphicsCardInfo() override;
    virtual void Render() override;
    virtual void Resize(const Extent2i& extent) override;
    virtual void Finalize() override;
private:
    void createVkInstance();
    void setDebugCallback();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createSwapChainImageViews();
    void createPipelineLayout();
    void createRenderPass();
    void createGraphicsPipeline();
    void createSwapChainFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
private:
    VkAllocationCallbacks*      m_allocator         = NULL;
    VkInstance                  m_instance          = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT    m_debugHandle       = VK_NULL_HANDLE;
    VkPhysicalDevice            m_physicalDevice    = VK_NULL_HANDLE;
    QueueFamilyIndices          m_queueFamily;
    VkDevice                    m_logicalDevice     = VK_NULL_HANDLE;
    VkQueue                     m_graphicsQueue     = VK_NULL_HANDLE;
    VkQueue                     m_presentQueue      = VK_NULL_HANDLE;
    VkSurfaceKHR                m_surface           = VK_NULL_HANDLE;
    // swap chain
    VkSwapchainKHR              m_swapChain         = VK_NULL_HANDLE;
    vector<VkImage>             m_swapChainImages;
    VkFormat                    m_swapChainFormat;
    VkExtent2D                  m_swapChainExtent;
    vector<VkImageView>         m_swapChainImageViews;
    vector<VkFramebuffer>       m_swapChainFramebuffers;
    // render pass
    VkRenderPass                m_renderPass        = VK_NULL_HANDLE;
    VkPipelineLayout            m_pipelineLayout    = VK_NULL_HANDLE;
    VkPipeline                  m_graphicsPipeline  = VK_NULL_HANDLE;
    // command buffer
    VkCommandPool               m_commandPool       = VK_NULL_HANDLE;
    vector<VkCommandBuffer>     m_commandBuffers;
};

} // namespace pbr
