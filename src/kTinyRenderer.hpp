#pragma once
#include <vulkan/vulkan.h>
//#include <GLFW/glfw3.h>

#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kGraphicsPipeline.hpp"
#include "kDescriptors.hpp"
#include "kModel.hpp"
#include "kCamera.hpp"


namespace engine
{
    class vkglTFModel;
    class TinyRenderer
    {
    public:
        TinyRenderer();
        ~TinyRenderer();

        TinyRenderer(const TinyRenderer&) = delete;
        TinyRenderer(TinyRenderer&& other) = delete;

        TinyRenderer& operator=(TinyRenderer&) = delete;
        TinyRenderer& operator=(TinyRenderer&&) = delete;

        void buildVulkan(uint32_t maxFramesInFlight, std::vector<karhu::vkglTFModel*>& models);
        void draw(uint32_t currentFrame, karhu::vkglTFModel *model);
        void update(uint32_t currentFrame, karhu::vkglTFModel* model);
        void recordCommandBuffer(VkCommandBuffer commandBuffer,
                uint32_t index, karhu::vkglTFModel *model);
        karhu::Vulkan_Device& getDeviceStruct() { return m_Device; }
    private:
        void createGraphicsPipeline();
        void createCommandPool();
        void createCommandBuffers(uint32_t maxFramesInFlight);
        void createDepthResources();
        void createFrameBuffers();
        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);
        void createUniformBuffers(uint32_t maxFramesInFlight);
        void createSyncObjects(uint32_t maxFramesinFlight);
        void cleanUpSwapChain();
        void reCreateSwapChain();
        void updateUBOs(uint32_t index, uint32_t currentFrame);
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    private:
        VkSurfaceKHR m_Surface;
        VkDescriptorSetLayout m_DescriptorLayout;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkCommandPool m_CommandPool;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        std::vector<VkFramebuffer> m_FrameBuffers;

        VkImage m_DepthImage;
        VkDeviceMemory m_DepthImageMemory;
        VkImageView m_DepthImageView;

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_UniformBuffersMapped;
        std::vector<VkFence> m_InFlightFences;

        struct m_Semaphores
        {
            std::vector<VkSemaphore> availableSemaphores;
            std::vector<VkSemaphore> finishedSemaphores;
        }m_Semaphores;

        uint32_t m_MaxFramesInFlight;

        VkDebugUtilsMessengerEXT m_DebugMessenger;

        std::unique_ptr<karhu::kWindow> m_Window = std::make_unique<karhu::kWindow>("Vulkan", 1080, 720);;
        karhu::Vulkan_Device m_Device{ m_Window->getInstance(), m_Window->getSurface() };
        karhu::Vulkan_SwapChain m_SwapChain{ m_Device };
        karhu::GraphicsPipeline m_Pipeline{ m_Device };
        karhu::kDescriptors m_Descriptors{ m_Device };
        karhu::Camera* m_Camera;
    };
} //engine