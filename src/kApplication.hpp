#pragma once
#include <vulkan/vulkan.h>
#include "kRenderer.hpp"
#include "kEntity.hpp"
#include "pipelines/kBasicRenderSystem.hpp"
#include "keyboardMovement.hpp"


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

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        
        void run();
        void update(float deltaTime, std::vector<VkDescriptorSet> sets, std::vector<std::unique_ptr<kBuffer>> uboBuffers);
        void renderEntities(kCamera& camera, uint32_t currentFrameIndex, uint32_t index);
    private:
        kRenderer m_Renderer{};
        kBasicRenderSystem entityPipeline{ m_Renderer.getDevice(),m_Renderer.getSwapChain(), m_Renderer.getGraphicsPipeLine() };
        kEntity::MAP m_Entities;

        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;

        std::unique_ptr<LveDescriptorPool> m_GlobalPool;
        std::vector<std::unique_ptr<LveDescriptorPool>> m_ObjectPools;

    };
} // namespace karhu