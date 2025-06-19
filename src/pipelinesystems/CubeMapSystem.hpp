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
    class CubeMapSystem
    {
        public:
            CubeMapSystem(Device& device);
            ~CubeMapSystem();
            
            CubeMapSystem(const CubeMapSystem&) = delete;
            CubeMapSystem &operator = (const CubeMapSystem&) = delete;

            void createDescriptors(Entity& entity);
            void createGraphicsPipeline(VkDevice device,
                    VkExtent2D extent,
                    VkDescriptorSetLayout layout,
                    VkRenderPass renderPass);
            void renderSkyBox(Frame& frameInfo, Entity& entity);

        private:
            CubePipelineBuilder m_pipelineBuilder;
            std::unique_ptr<Descriptors> m_descriptorBuilder;

            VkDescriptorPool m_pool;
            VkDescriptorSetLayout m_layout;
            std::vector<VkDescriptorSetLayoutBinding> m_bindings;


            Device& m_device;
    };
} // karhu namespace
