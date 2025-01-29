#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kGraphicsPipeline.hpp"
#include "kModel.hpp"
#include "kDescriptors.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{


    class Application
    {
    public:
        Application();
        ~Application();
        void run();
        void createFrameBuffers();
        void createCommandPool();
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index);
        void createSyncObjects();
        void createUniformBuffers();
        void updateUBOs(uint32_t currentImage);
        void update(float deltaTime);
        void drawFrame();
    private:
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        void createGraphicsPipeline();
        void cleanUpSwapChain();
        void reCreateSwapChain();
    private:
        std::unique_ptr<kWindow> m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
        Vulkan_Device m_VkDevice{ m_Window->getInstance(), m_Window->getSurface() };
        Vulkan_SwapChain m_VkSwapChain{ m_VkDevice };
        kGraphicsPipeline m_GraphicsPipeline{ m_VkDevice };
        std::unique_ptr<kModel> m_Model;
        std::shared_ptr<kDescriptors> m_Descriptor; /*= std::make_shared<kDescriptors>();*/


        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkDescriptorSetLayout m_DescriptorLayout;
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

        const std::vector<Vertex> m_Vertices = {
            {{-0.5f,-0.5f},{1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f},{0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f},{0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f},{1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint16_t> m_Indices = {
            0,1,2,2,3,0
        };

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_UniformBuffersMapped;

        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };
} // namespace karhu
