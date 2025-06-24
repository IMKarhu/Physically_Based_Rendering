#pragma once

#include "../PipelineBuilder.hpp"
#include "../frame.hpp"
#include "../FrameBuffer.hpp"
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include <vector>
#include <memory>


namespace karhu
{
    class Device;
    class Descriptors;
    class Entity;
    class CommandBuffer;
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
            void updateCubeUbo();

            void generateBrdfLut(VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffer, CommandBuffer& commandBuffer);
            void generateIrradianceCube(VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffer, CommandBuffer& commandBuffer);
            void generatePreFilteredCube();
        private:
            CubePipelineBuilder m_pipelineBuilder;
            CubePipelineBuilder m_brdflutPipelineBuilder;
            std::unique_ptr<Descriptors> m_descriptorBuilder;
            std::unique_ptr<Descriptors> m_brdflutBuilder;

            VkDescriptorPool m_pool;
            VkDescriptorSetLayout m_layout;
            std::vector<VkDescriptorSetLayoutBinding> m_bindings;
            std::vector<glm::mat4> m_matrices;

            struct Textures {
                Image m_brdfLut;
                
                Image m_irradianceCube;
            } m_textures;


            Device& m_device;
    };
} // karhu namespace
