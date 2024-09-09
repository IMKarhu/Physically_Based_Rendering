#pragma once
#include "kTinyRenderer.hpp"
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kDescriptors.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{
    class vkglTFModel;
    class Application
    {
    public:
        Application();
        ~Application();
        void run();
        void buildVulkan();
        void createGraphicsPipeline();
        void createRenderPass();
        void createFrameBuffers();
        void createCommandPool();
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index);
        void createSyncObjects();
        void createUniformBuffers();
        void createDepthResources();
        void updateUBOs(uint32_t currentImage);
        void update(float deltaTime);
        void drawFrame();
    private:
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        void createSurface();
        VkSwapchainCreateInfoKHR fillSwapchainCI();
        static std::vector<char> readFile(const std::string& fileName);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void cleanUpSwapChain();
        void reCreateSwapChain();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
        bool hasStencilComponent(VkFormat format);
        void loadGltfFile(std::string fileName);
    private:
        //std::unique_ptr<kWindow> m_Window;
       /* std::shared_ptr<Vulkan_Device> m_VkDevice;
        std::shared_ptr<Vulkan_SwapChain> m_VkSwapChain;
        std::shared_ptr<kDescriptors> m_Descriptor;*/
        std::vector<vkglTFModel *> m_Models;

        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
        VkRenderPass m_RenderPass;
        VkDescriptorSetLayout m_DescriptorLayout;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
        std::vector<VkDynamicState> m_DynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        std::vector<VkFramebuffer> m_FrameBuffers;
        VkCommandPool m_CommandPool;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        struct m_Semaphores
        {
            std::vector<VkSemaphore> availableSemaphores;
            std::vector<VkSemaphore> finishedSemaphores;
        }m_Semaphores;
        std::vector<VkFence> m_InFlightFences;
        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;
        const int m_MaxFramesInFlight = 2;

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_UniformBuffersMapped;

        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        

        VkImage m_DepthImage;
        VkDeviceMemory m_DepthImageMemory;
        VkImageView m_DepthImageView;
        engine::TinyRenderer* m_Renderer;
    };
} // namespace karhu
