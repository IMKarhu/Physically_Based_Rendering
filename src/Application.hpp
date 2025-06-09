#pragma once

#include "Window.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "pipelinesystems/DisneySystem.hpp"
#include "RenderPass.hpp"
#include "Image.hpp"

#include <memory>

namespace karhu
{
    class Application
    {
        public:
            Application();
            ~Application();

            void run();
            void update();
            void begin(uint32_t currentFrameIndex, uint32_t imageIndex);
            void end(uint32_t currentFrameIndex, uint32_t imageIndex);
        private:
            void createSyncObjects();
        private:
            std::unique_ptr<Window> m_window = std::make_unique<Window>("Vulkan", 1080, 720);
            Device m_device{ m_window->getInstance(), m_window->getSurface() };
            CommandBuffer m_commandBuffer{ m_device };
            SwapChain m_swapChain{ m_device, m_window };
            DisneySystem m_disneySystem;
            std::vector<RenderPass> m_renderPasses;
            std::vector<Image> m_images;

            std::vector<VkFramebuffer> m_framebuffers;

            /*synchronization*/
            struct Semaphoras
            {
                std::vector<VkSemaphore> m_availableSemaphores;
                std::vector<VkSemaphore> m_finishedSemaphores;
            }m_semaphores;
            std::vector<VkFence> m_inFlightFences;
            uint32_t m_currentImage = 0;
            float m_deltaTime = 0.0f;
    };
} // karhu namespace
