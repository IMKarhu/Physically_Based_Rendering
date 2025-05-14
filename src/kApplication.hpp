#pragma once
#include "kRenderer.hpp"
#include "keyboardMovement.hpp"
#include "kEntity.hpp"
#include "pipelines/kBasicRenderSystem.hpp"
#include "pipelines/kUnrealRenderSystem.hpp"

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
        void update(float deltaTime, std::vector<std::unique_ptr<kBuffer>>& buffers);
    private:
        kRenderer m_Renderer{};
        kDescriptors m_GlobalDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_ObjDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_UnrealObjDescriptorBuilder{ m_Renderer.getDevice() };
        kBasicRenderSystem m_EntityPipeline{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        kUnrealRenderSystem m_UnrealEntityPipeline{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        std::vector<kEntity> m_Entities;
        std::vector<kEntity> m_UnrealEntities;

        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkDescriptorSetLayoutBinding> m_GlobalBindings;
        VkDescriptorSetLayout m_GlobalLayout;
        VkDescriptorPool m_GlobalPool;
        VkDescriptorSet m_GlobalSet;

        VkDescriptorPool m_ObjPool;
        VkDescriptorSetLayout m_ObjLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_ObjBindings;

        VkDescriptorPool m_UnrealObjPool;
        VkDescriptorSetLayout m_UnrealObjLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_UnrealObjBindings;
    };
} // namespace karhu
