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
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index);
        void createSyncObjects();
        void createUniformBuffers();
        void updateUBOs(uint32_t currentImage);
        void update(float deltaTime);
        void drawFrame();
    private:

        void createGraphicsPipeline();
        void createDepthResources(); //refactor somewhere else image class?
        VkFormat findDepthFormat(); // refactor somewhere elseimage class?
        bool hasStencilComponent(VkFormat format); //refactor somewhere else iamge class?
        void createImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image, VkDeviceMemory& imageMemory); //reafctor somewhere else image class?
        void cleanUpSwapChain();
        void reCreateSwapChain();
    private:
        std::unique_ptr<kWindow> m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
        Vulkan_Device m_VkDevice{ m_Window->getInstance(), m_Window->getSurface() };
        Vulkan_SwapChain m_VkSwapChain{ m_VkDevice };
        kGraphicsPipeline m_GraphicsPipeline{ m_VkDevice };
        std::unique_ptr<kModel> m_Model;
        std::shared_ptr<kDescriptors> m_Descriptor; /*= std::make_shared<kDescriptors>();*/

        VkDescriptorSetLayout m_DescriptorLayout;
        std::vector<VkFramebuffer> m_FrameBuffers;
        struct m_Semaphores
        {
            std::vector<VkSemaphore> availableSemaphores;
            std::vector<VkSemaphore> finishedSemaphores;
        }m_Semaphores;
        std::vector<VkFence> m_InFlightFences;
        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;
        

        const std::vector<Vertex> m_Vertices = {
            {{-0.5f,-0.5f, 0.0f},{1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f},{0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f},{0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f},{1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint16_t> m_Indices = {
            0,1,2,2,3,0,
            4,5,6,6,7,4
        };

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_UniformBuffersMapped;

        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;

        VkImage m_DepthImage;
        VkDeviceMemory m_DepthImageMemory;
        VkImageView m_DepthImageView;
    };
} // namespace karhu
