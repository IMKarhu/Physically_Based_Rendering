#include "DisneySystem.hpp"

#include "../Device.hpp"
#include "../Descriptors.hpp"
#include "../Entity.hpp"

namespace karhu
{
    DisneySystem::DisneySystem(Device& device)
        : m_device(device)
    {
    }
    
    DisneySystem::~DisneySystem()
    {
    }

    void DisneySystem::createDescriptors(std::vector<Entity>& entities, std::vector<Entity>& spheres)
    {
        m_descriptorBuilder = std::make_unique<Descriptors>(m_device);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_descriptorBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_pool = m_descriptorBuilder->createDescriptorPool(1000);

        m_descriptorBuilder->bind(m_bindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_descriptorBuilder->bind(m_bindings, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_layout = m_descriptorBuilder->createDescriptorSetLayout(m_bindings);

        std::vector<std::vector<VkDescriptorImageInfo>> infos;
        infos.resize(entities.size());
        for (size_t i = 0; i < entities.size(); i++)
        {
            for (size_t j = 0; j < entities[i].getModel()->m_Textures.size(); j++)
            {
                // auto tex = static_cast<karhu::NTexture>(entities[i].getModel()->m_Textures[j]);
                infos[i].push_back(entities[i].getModel()->m_Textures[j].getImageInfo());
            }
            entities[i].m_Buffer = std::make_unique<Buffer>();
            entities[i].m_Buffer->m_device = m_device.lDevice();
            entities[i].m_Buffer->m_phyiscalDevice = m_device.pDevice();
            entities[i].m_Buffer->createBuffer(sizeof(ObjBuffer),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        }

        for (size_t i = 0; i < entities.size(); i++)
        {
            auto id = entities[i].getId();
            m_descriptorBuilder->allocateDescriptor(entities[i].m_DescriptorSet, m_layout, m_pool);
            m_descriptorBuilder->writeBuffer(entities[i].m_DescriptorSet,
                    0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, entities[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);
         
            m_descriptorBuilder->writeImg(entities[i].m_DescriptorSet, 1, infos[i][0], id);
            m_descriptorBuilder->writeImg(entities[i].m_DescriptorSet, 2, infos[i][1], id);
            m_descriptorBuilder->writeImg(entities[i].m_DescriptorSet, 3, infos[i][2], id);
            m_descriptorBuilder->writeImg(entities[i].m_DescriptorSet, 4, infos[i][3], id);
            m_descriptorBuilder->writeImg(entities[i].m_DescriptorSet, 5, infos[i][4], id);
            m_descriptorBuilder->fillWritesMap(entities[i].getId());
        }

        m_descriptorBuilder->createDescriptorSets(entities, m_layout, m_pool);

        m_sphereDescBuilder = std::make_unique<Descriptors>(m_device);
        m_sphereDescBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_pool1 = m_sphereDescBuilder->createDescriptorPool(1000);

        m_sphereDescBuilder->bind(m_bindings1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_layout1 = m_sphereDescBuilder->createDescriptorSetLayout(m_bindings1);

        for (size_t i = 0; i < spheres.size(); i++)
        {
            spheres[i].m_Buffer = std::make_unique<Buffer>();
            spheres[i].m_Buffer->m_device = m_device.lDevice();
            spheres[i].m_Buffer->m_phyiscalDevice = m_device.pDevice();
            spheres[i].m_Buffer->createBuffer(sizeof(ObjBuffer),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        for (size_t i = 0; i < spheres.size(); i++)
        {
            auto id = spheres[i].getId();
            m_sphereDescBuilder->allocateDescriptor(spheres[i].m_DescriptorSet, m_layout1, m_pool1);
            m_sphereDescBuilder->writeBuffer(spheres[i].m_DescriptorSet,
                    0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, spheres[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);
            m_sphereDescBuilder->fillWritesMap(spheres[i].getId());
        }

        m_sphereDescBuilder->createDescriptorSets(spheres, m_layout1, m_pool1);

    }

    void DisneySystem::createGraphicsPipeline(VkDevice device,
            VkExtent2D extent,
            VkDescriptorSetLayout layout,
            VkRenderPass renderPass)
    {
        std::vector<VkDescriptorSetLayout> layouts = {
            layout,
            m_layout
        };

        m_pipelinebuilder.getHandle()->m_device = device;

        PipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = extent.width;
        pipelineStruct.viewportheight = extent.height;
        pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineStruct.renderPass = renderPass;

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        VkPushConstantRange objPushConstant{};
        objPushConstant.offset = 0;
        objPushConstant.size = sizeof(ObjPushConstant);
        objPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPushConstantRange cameraPushConstant{};
        cameraPushConstant.offset = 64;
        cameraPushConstant.size = sizeof(pushConstants);
        cameraPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkPushConstantRange> pushConstantRanges{
            objPushConstant,
            cameraPushConstant
        };

        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()); // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data(); // Optional


        m_pipelinebuilder.createPipeline(pipelineStruct, "../shaders/vertexShader.spv", "../shaders/fragmentShader.spv");

        layouts = {
            layout,
            m_layout1
        };

        m_spherePipeline.getHandle()->m_device = device;

        pipelineStruct.viewportWidth = extent.width;
        pipelineStruct.viewportheight = extent.height;
        pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineStruct.renderPass = renderPass;

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        objPushConstant.offset = 0;
        objPushConstant.size = sizeof(ObjPushConstant);
        objPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        cameraPushConstant.offset = 64;
        cameraPushConstant.size = sizeof(pushConstants);
        cameraPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()); // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data(); // Optional

        m_spherePipeline.createPipeline(pipelineStruct, "../shaders/NoTexvert.spv", "../shaders/NoTexfrag.spv");
    }

    void DisneySystem::renderEntities(Frame& frameInfo)
    {
        m_pipelinebuilder.bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelinebuilder.getHandle()->m_pipelineLayout,
                0,
                1,
                &frameInfo.globalSet,
                0,
                nullptr);

        for (auto& entity : frameInfo.entities)
        {
            vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipelinebuilder.getHandle()->m_pipelineLayout,
                    1,
                    1,
                    &entity.m_DescriptorSet,
                    0,
                    nullptr);
        
            ObjPushConstant objConstant{};
            objConstant.model = entity.getTransformMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer,
                    m_pipelinebuilder.getHandle()->m_pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(ObjPushConstant),
                    &objConstant);
        
            pushConstants cameraConstants{};
            cameraConstants.cameraPosition = frameInfo.camera.getPosition();
            cameraConstants.lightPosition = vars.m_LightPosition;
            cameraConstants.lighColor = vars.m_lightColor;
            cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
            vkCmdPushConstants(frameInfo.commandBuffer,
                    m_pipelinebuilder.getHandle()->m_pipelineLayout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    64,
                    sizeof(pushConstants),
                    &cameraConstants);

            entity.getModel()->bind(frameInfo.commandBuffer);
            entity.getModel()->draw(frameInfo.commandBuffer);
        }
    }

    void DisneySystem::renderEntitiesNotextures(Frame& frameInfo)
    {
        m_spherePipeline.bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_spherePipeline.getHandle()->m_pipelineLayout,
                0,
                1,
                &frameInfo.globalSet,
                0,
                nullptr);

        float met = 0.0f;
        for (auto& entity : frameInfo.spheres)
        {
            met = vars.m_Metalness + 0.1f;
            vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_spherePipeline.getHandle()->m_pipelineLayout,
                    1,
                    1,
                    &entity.m_DescriptorSet,
                    0,
                    nullptr);
        
            ObjPushConstant objConstant{};
            objConstant.model = entity.getTransformMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer,
                    m_spherePipeline.getHandle()->m_pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(ObjPushConstant),
                    &objConstant);
        
            pushConstants cameraConstants{};
            cameraConstants.cameraPosition = frameInfo.camera.getPosition();
            cameraConstants.lightPosition = vars.m_LightPosition;
            cameraConstants.lighColor = vars.m_lightColor;
            cameraConstants.albedoNormalMetalRoughness = glm::vec4(met, vars.m_Roughness,0.0f,0.0f);
            vkCmdPushConstants(frameInfo.commandBuffer,
                    m_spherePipeline.getHandle()->m_pipelineLayout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    64,
                    sizeof(pushConstants),
                    &cameraConstants);

            entity.getModel()->bind(frameInfo.commandBuffer);
            entity.getModel()->draw(frameInfo.commandBuffer);
            vars.m_Metalness += 0.1f;
            // vars.m_Roughness += 0.1f;
        }

    }
}
