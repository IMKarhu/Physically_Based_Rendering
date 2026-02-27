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
        
        void createDescriptors(std::vector<Entity>& entities, std::vector<Entity>& spheres);
        void createGraphicsPipeline(VkDevice device,
                VkExtent2D extent,
                VkDescriptorSetLayout layout,
                VkRenderPass renderPass);
        void renderEntities(Frame& frameInfo);
        void renderEntitiesNotextures(Frame& frameInfo);
        bool isIbl() { return vars.ibl; }
        bool isAlbedo() { return vars.albedo; }
        bool isPhong() { return vars.phong; }
        bool isBlinnPhong() { return vars.blinnPhong; }


        void setIblActive(bool ibl);
        void setAlbedoActive(bool albedo);
        void setPhongActive(bool phong);
        void setBlinnPhongActive(bool blinnphong);
    private:
        NormalPipelineBuilder m_pipelinebuilder;
        NormalPipelineBuilder m_spherePipeline;
        std::unique_ptr<Descriptors> m_descriptorBuilder;
        std::unique_ptr<Descriptors> m_sphereDescBuilder;

        VkDescriptorPool m_pool;
        VkDescriptorSetLayout m_layout;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;

        VkDescriptorPool m_pool1;
        VkDescriptorSetLayout m_layout1;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings1;

        struct {
            float m_Metalness = 0.0f;
            float m_Roughness = 0.025f;
            float offset = 0.0f;
            bool ibl = false;
            bool albedo = false;
            bool phong = false;
            bool blinnPhong = false;
            glm::vec3 m_LightPosition = glm::vec3(0.0f, 10.0f, 0.0f); // 200 y
            glm::vec4 m_lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }vars;

        Device& m_device;
    };
}
