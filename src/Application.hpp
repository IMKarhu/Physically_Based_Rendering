#pragma once

#include "Window.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "pipelinesystems/DisneySystem.hpp"
#include "RenderPass.hpp"
#include "Image.hpp"
#include "Descriptors.hpp"
#include "Entity.hpp"
#include "Camera.hpp"

#include <memory>
#include <unordered_map>

namespace karhu
{
    class Application
    {
        public:
            Application();
            ~Application();

            enum enityType
            {
                Disney,
                Unreal,
                Cube
            };

            void run();
            void update(std::vector<std::unique_ptr<Buffer>>& gBuffers);
            void begin(uint32_t currentFrameIndex, uint32_t imageIndex);
            void end(uint32_t currentFrameIndex, uint32_t imageIndex);
        private:
            void updateBuffers(std::vector<std::unique_ptr<Buffer>>& gBuffers, Camera& camera);
            void createSyncObjects();
            void cleanUpBeforeReCreate();
            void reCreateSwapChain();
        private:
            std::unique_ptr<Window> m_window = std::make_unique<Window>("Vulkan", 1080, 720);
            Device m_device{ m_window->getInstance(), m_window->getSurface() };
            CommandBuffer m_commandBuffer{ m_device };
            SwapChain m_swapChain{ m_device, m_window };
            DisneySystem m_disneySystem{ m_device };
            Descriptors m_builder{ m_device };
            std::vector<RenderPass> m_renderPasses;
            Image m_depthImage;

            std::vector<VkFramebuffer> m_framebuffers;

            std::unordered_map<enityType, std::vector<Entity>> m_entities;

            /*synchronization*/
            struct Semaphoras
            {
                std::vector<VkSemaphore> m_availableSemaphores;
                std::vector<VkSemaphore> m_finishedSemaphores;
            }m_semaphores;
            std::vector<VkFence> m_inFlightFences;
            uint32_t m_currentImage = 0;
            uint32_t m_currentFrame = 0;
            float m_deltaTime = 0.0f;

            /*Camera descriptor.... global...*/
            std::vector<VkDescriptorSetLayoutBinding> m_bindings;
            VkDescriptorSetLayout m_layout;
            VkDescriptorPool m_pool;
            VkDescriptorSet m_set;
    };
} // karhu namespace
