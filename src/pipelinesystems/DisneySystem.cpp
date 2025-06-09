#include "DisneySystem.hpp"

namespace karhu
{
    DisneySystem::DisneySystem()
    {
    }
    
    DisneySystem::~DisneySystem()
    {
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
