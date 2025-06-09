#include "DisneySystem.hpp"

#include "../Device.hpp"
#include "../Descriptors.hpp"

namespace karhu
{
    DisneySystem::DisneySystem(Device& device)
        : m_device(device)
    {
    }
    
    DisneySystem::~DisneySystem()
    {
    }

    void DisneySystem::createDescriptors()
    {
        m_descriptorBuilder = std::make_unique<Descriptors>(m_device);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_pool = m_descriptorBuilder->createDescriptorPool(1000);

        m_descriptorBuilder->bind(m_bindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_descriptorBuilder->bind(m_bindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_layout = m_descriptorBuilder->createDescriptorSetLayout(m_bindings);

        std::vector<std::vector<VkDescriptorImageInfo>> infos;
        infos.resize(m_Entities.size());
        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            for (size_t j = 0; j < m_Entities[i].getModel()->m_Textures.size(); j++)
            {
                infos[i].push_back(m_Entities[i].getModel()->m_Textures[j].getImageInfo());
            }
            m_Entities[i].m_Buffer = std::make_unique<kBuffer>(m_device.lDevice());
            m_Entities[i].m_Buffer->createBuffer(sizeof(ObjBuffer));
        }

        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            auto id = m_Entities[i].getId();
            m_descriptorBuilder->allocateDescriptor(m_Entities[i].m_DescriptorSet, m_layout, m_pool);
            m_descriptorBuilder->writeBuffer(m_Entities[i].m_DescriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_Entities[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);
         
            m_descriptorBuilder->writeImg(m_Entities[i].m_DescriptorSet, 1, infos[i][0], id);
            m_descriptorBuilder->writeImg(m_Entities[i].m_DescriptorSet, 2, infos[i][1], id);
            m_descriptorBuilder->writeImg(m_Entities[i].m_DescriptorSet, 3, infos[i][2], id);
            m_descriptorBuilder->writeImg(m_Entities[i].m_DescriptorSet, 4, infos[i][3], id);
            m_descriptorBuilder->writeImg(m_Entities[i].m_DescriptorSet, 5, infos[i][4], id);
            m_descriptorBuilder->fillWritesMap(m_Entities[i].getId());
            
        }
    }

    void DisneySystem::createGraphicsPipeline(VkDevice device,
            VkExtent2D extent,
            std::vector<VkDescriptorSetLayout> layouts,
            VkRenderPass renderPass)
    {
        m_pipelinebuilder.getHandle()->m_device = device;

        PipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = extent.width;
        pipelineStruct.viewportheight = extent.height;
        pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineStruct.renderPass = renderPass;

        m_pipelinebuilder.createPipeline(pipelineStruct, "../shaders/vertexShader.spv", "../shaders/fragmentShader.spv");
    }

    void DisneySystem::renderEntities(Frame& frameInfo)
    {
        // m_EntityPipeline->bind(frameInfo.commandBuffer);
        
        // vkCmdBindDescriptorSets(frameInfo.commandBuffer,
        //         VK_PIPELINE_BIND_POINT_GRAPHICS,
        //         m_EntityPipeline->getPipelineLayout(),
        //         0,
        //         1,
        //         &frameInfo.globalSet,
        //         0,
        //         nullptr);
        //
        // for (auto& entity : frameInfo.entities)
        // {
        //
        //     vkCmdBindDescriptorSets(frameInfo.commandBuffer,
        //             VK_PIPELINE_BIND_POINT_GRAPHICS,
        //             m_EntityPipeline->getPipelineLayout(),
        //             1,
        //             1,
        //             &entity.m_DescriptorSet,
        //             0,
        //             nullptr);
        //
        //     ObjPushConstant objConstant{};
        //     objConstant.model = entity.getTransformMatrix();
        //     vkCmdPushConstants(frameInfo.commandBuffer,
        //             m_EntityPipeline->getPipelineLayout(),
        //             VK_SHADER_STAGE_VERTEX_BIT,
        //             0,
        //             sizeof(ObjPushConstant),
        //             &objConstant);
        //
        //     pushConstants cameraConstants{};
        //     cameraConstants.cameraPosition = cameraPos;
        //     cameraConstants.lightPosition = vars.m_LightPosition;
        //     cameraConstants.lighColor = lightColor;
        //     cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
        //     vkCmdPushConstants(frameInfo.commandBuffer,
        //             m_EntityPipeline->getPipelineLayout(),
        //             VK_SHADER_STAGE_FRAGMENT_BIT,
        //             64,
        //             sizeof(pushConstants),
        //             &cameraConstants);
        //
        //
        //     entity.getModel()->bind(frameInfo.commandBuffer);
        //     entity.getModel()->draw(frameInfo.commandBuffer);
        // }
    }
}
