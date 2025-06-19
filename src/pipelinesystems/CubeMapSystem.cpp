#include "CubeMapSystem.hpp"

#include "../Device.hpp"
#include "../Entity.hpp"
#include "../Descriptors.hpp"

namespace karhu
{
    CubeMapSystem::CubeMapSystem(Device& device)
        : m_device(device) {}

    CubeMapSystem::~CubeMapSystem() {}

    void CubeMapSystem::createDescriptors(Entity& entity)
    {
        m_descriptorBuilder = std::make_unique<Descriptors>(m_device);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_pool = m_descriptorBuilder->createDescriptorPool(1000);

        m_descriptorBuilder->bind(m_bindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_descriptorBuilder->bind(m_bindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_layout = m_descriptorBuilder->createDescriptorSetLayout(m_bindings);

        std::vector<VkDescriptorImageInfo> infos;
        infos.resize(1);
        infos.push_back(entity.getModel()->m_Textures[0].getImageInfo());
        entity.m_Buffer = std::make_unique<Buffer>();
        entity.m_Buffer->m_device = m_device.lDevice();
        entity.m_Buffer->m_phyiscalDevice = m_device.pDevice();
        entity.m_Buffer->createBuffer(sizeof(ObjBuffer),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        auto id = entity.getId();
        m_descriptorBuilder->allocateDescriptor(entity.m_DescriptorSet, m_layout, m_pool);
        m_descriptorBuilder->writeBuffer(entity.m_DescriptorSet,
                0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, entity.m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);
        
        m_descriptorBuilder->writeImg(entity.m_DescriptorSet, 1, infos[0], id);
        m_descriptorBuilder->fillWritesMap(entity.getId());

        m_descriptorBuilder->createDescriptorSets(m_layout, m_pool);

    }
    void CubeMapSystem::createGraphicsPipeline(VkDevice device,
                VkExtent2D extent,
                VkDescriptorSetLayout layout,
                VkRenderPass renderPass)
    {
        std::vector<VkDescriptorSetLayout> layouts = {
            layout,
            m_layout
        };

        m_pipelineBuilder.getHandle()->m_device = device;

        PipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = extent.width;
        pipelineStruct.viewportheight = extent.height;
        pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineStruct.renderPass = renderPass;

        m_pipelineBuilder.createPipeline(pipelineStruct, "../shaders/cubemapvert.spv", "../shaders/cubemapfrag.spv");
    }

    void CubeMapSystem::renderSkyBox(Frame& frameInfo, Entity& entity)
    {
        m_pipelineBuilder.bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineBuilder.getHandle()->m_pipelineLayout,
                0,
                1,
                &frameInfo.globalSet,
                0,
                nullptr);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineBuilder.getHandle()->m_pipelineLayout,
                1,
                1,
                &entity.m_DescriptorSet,
                0,
                nullptr);
        
        ObjPushConstant objConstant{};
        objConstant.model = entity.getTransformMatrix();
        vkCmdPushConstants(frameInfo.commandBuffer,
                m_pipelineBuilder.getHandle()->m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(ObjPushConstant),
                &objConstant);
        
        pushConstants cameraConstants{};
        cameraConstants.cameraPosition = frameInfo.camera.getPosition();
        cameraConstants.lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraConstants.lighColor = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f);
        cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        vkCmdPushConstants(frameInfo.commandBuffer,
                m_pipelineBuilder.getHandle()->m_pipelineLayout,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                64,
                sizeof(pushConstants),
                &cameraConstants);
        
        entity.getModel()->bind(frameInfo.commandBuffer);
        entity.getModel()->draw(frameInfo.commandBuffer);
    }
} // karhu namespace
