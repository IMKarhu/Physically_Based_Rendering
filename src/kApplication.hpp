#pragma once
#include "kRenderer.hpp"
#include "keyboardMovement.hpp"
#include "kEntity.hpp"

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
        //std::unique_ptr<kModel> m_Model;
        std::vector<kEntity> m_Entities;

        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkDescriptorSetLayoutBinding> m_GlobalBindings;
        VkDescriptorSetLayout m_GlobalLayout;
        VkDescriptorPool m_GlobalPool;
        VkDescriptorSet m_GlobalSet;

        VkDescriptorPool m_ObjPool;
        VkDescriptorSetLayout m_ObjLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_ObjBindings;
    };
} // namespace karhu
