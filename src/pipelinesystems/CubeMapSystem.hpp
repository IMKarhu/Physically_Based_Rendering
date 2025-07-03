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
    class Camera;
    class CubeMapSystem
    {
        public:
            CubeMapSystem(Device& device);
            ~CubeMapSystem();
            
            CubeMapSystem(const CubeMapSystem&) = delete;
            CubeMapSystem &operator = (const CubeMapSystem&) = delete;

            //passing in IBL textures just for testing if convolution works
            void createDescriptors(Entity& entity, IblTextures& textures);
            void createGraphicsPipeline(VkDevice device,
                    VkExtent2D extent,
                    VkDescriptorSetLayout layout,
                    VkRenderPass renderPass);
            void renderSkyBox(Frame& frameInfo, Entity& entity);
            void updateCubeUbo(Entity& entity, Camera& camera);

            void generateBrdfLut(VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffer, CommandBuffer& commandBuffer, IblTextures& textures);
            void generateIrradianceCube(VkRenderPass renderPass,
                    std::vector<VkFramebuffer>& frameBuffer,
                    CommandBuffer& commandBuffer,
                    Entity& entity,
                    IblTextures& textures);
            void generatePreFilteredCube(VkRenderPass renderPass,
            std::vector<VkFramebuffer>& frameBuffer,
            CommandBuffer& commandBuffer,
            Entity& entity,
            IblTextures& textures);
        private:
            CubePipelineBuilder m_pipelineBuilder;
            CubePipelineBuilder m_brdflutPipelineBuilder;
            CubePipelineBuilder m_irradiancePipelineBuilder;
            CubePipelineBuilder m_prefilteredPipelineBuilder;
            std::unique_ptr<Descriptors> m_descriptorBuilder;
            std::unique_ptr<Descriptors> m_brdflutBuilder;
            std::unique_ptr<Descriptors> m_irradianceCubeBuilder;
            std::unique_ptr<Descriptors> m_prefilteredCubeBuilder;

            VkDescriptorPool m_pool;
            VkDescriptorSetLayout m_layout;
            std::vector<VkDescriptorSetLayoutBinding> m_bindings;
            std::vector<glm::mat4> m_matrices;
            std::unique_ptr<Buffer> m_uboBuffer;

            struct Textures {
                Image m_brdfLut;
                
                Image m_irradianceCube;
            } m_textures;

            struct Params
            {
                glm::mat4 proj;
                glm::mat4 view;
                float exposure = 4.5f;
                float gamma = 2.2f;
            } m_params;

            static constexpr double M_PI = 3.14159265358979323846;

            Device& m_device;
    };
} // karhu namespace
