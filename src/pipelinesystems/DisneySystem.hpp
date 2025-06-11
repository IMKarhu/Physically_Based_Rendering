#pragma once

#include "../PipelineBuilder.hpp"
#include "../frame.hpp"
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include <vector>
#include <memory>

namespace karhu
{
    class Device;
    class Descriptors;
    class Entity;
    class DisneySystem
    {
    public:
        DisneySystem(Device& device);
        ~DisneySystem();
        
        DisneySystem(const DisneySystem&) = delete;
        DisneySystem &operator = (const DisneySystem&) = delete;
        
        void createDescriptors(std::vector<Entity>& entities);
        void createGraphicsPipeline(VkDevice device,
                VkExtent2D extent,
                VkDescriptorSetLayout layout,
                VkRenderPass renderPass);
        void renderEntities(Frame& frameInfo);
    private:
        NormalPipelineBuilder m_pipelinebuilder;
        std::unique_ptr<Descriptors> m_descriptorBuilder;

        VkDescriptorPool m_pool;
        VkDescriptorSetLayout m_layout;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;

        struct {
            float m_Metalness = 0.0f;
            float m_Roughness = 0.0f;
            glm::vec3 m_LightPosition = glm::vec3(1.0f, 3.0f, 1.0f);
            glm::vec4 m_lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }vars;

        Device& m_device;
    };
}
