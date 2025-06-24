#include "CubeMapSystem.hpp"

#include "../Device.hpp"
#include "../Entity.hpp"
#include "../Descriptors.hpp"
#include "../FrameBuffer.hpp"
#include "../CommandBuffer.hpp"

namespace karhu
{
    CubeMapSystem::CubeMapSystem(Device& device)
        : m_device(device)
    {
        m_matrices = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // +X
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // -X
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),  // +Y
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),  // -Y
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // +Z
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))   // -Z
        };
    }

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

    void CubeMapSystem::updateCubeUbo()
    {
    }

    void CubeMapSystem::generateBrdfLut(VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffer, CommandBuffer& commandBuffer)
    {
        printf("Starting BRDF lookup table generation\n");
        const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        const uint32_t dimensions = 512;

        m_textures.m_brdfLut = Image(m_device.lDevice(),
                m_device.pDevice(),
                1,
                dimensions,
                dimensions,
                format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_textures.m_brdfLut.createSampler(m_device.lDevice(), 1);

        karhu::createFrameBuffer1(m_device.lDevice(),
                frameBuffer,
                m_textures.m_brdfLut.getImageView(),
                renderPass,
                dimensions,
                dimensions,
                1);

        m_brdflutBuilder = std::make_unique<Descriptors>(m_device);
        VkDescriptorSetLayout layout;
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        layout = m_brdflutBuilder->createDescriptorSetLayout(bindings);
        VkDescriptorPool pool;
        m_brdflutBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        pool = m_brdflutBuilder->createDescriptorPool(2);

        VkDescriptorSet set;
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = pool;
        descriptorSetAllocateInfo.pSetLayouts = &layout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        VK_CHECK(vkAllocateDescriptorSets(m_device.lDevice(), &descriptorSetAllocateInfo, &set));

        m_brdflutPipelineBuilder.getHandle()->m_device = m_device.lDevice();

        PipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = dimensions;
        pipelineStruct.viewportheight = dimensions;
        // pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = 1;
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = &layout;
        pipelineStruct.renderPass = renderPass;

        m_brdflutPipelineBuilder.createPipeline(pipelineStruct, "../shaders/brdflutvert.spv", "../shaders/brdflutfrag.spv");

        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = renderPass;
        beginInfo.renderArea.extent.width = dimensions;
        beginInfo.renderArea.extent.height = dimensions;
        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = clearValues;
        beginInfo.framebuffer = frameBuffer[0];

        VkCommandBuffer cmdBuf;
        commandBuffer.allocCommandBuffer(cmdBuf);
        commandBuffer.beginCommand(cmdBuf);

        vkCmdBeginRenderPass(cmdBuf, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = dimensions;
        viewPort.height = dimensions;
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(cmdBuf, 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = dimensions;
        scissor.extent.height = dimensions;

        vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

        m_brdflutPipelineBuilder.bind(cmdBuf);

        vkCmdDraw(cmdBuf, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmdBuf);
        vkEndCommandBuffer(cmdBuf);


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuf;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        vkCreateFence(m_device.lDevice(), &fenceInfo, nullptr, &fence);
        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitInfo, fence));
        vkWaitForFences(m_device.lDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
        commandBuffer.flushCommandBuffer(cmdBuf);

        printf("Lookup table generated!\n");
    }

    void CubeMapSystem::generateIrradianceCube(VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffer, CommandBuffer& commandBuffer)
    {
        const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
        const uint32_t dimensions = 64;
        const uint32_t mips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

        m_textures.m_irradianceCube = Image(m_device.lDevice(),
                m_device.pDevice(),
                mips,
                dimensions,
                dimensions,
                format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true);
        m_textures.m_irradianceCube.createSampler(m_device.lDevice(), mips);


        struct {
            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;
            VkFramebuffer frameBuffer;
        } offScreen;

        {
            // Color attachment
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.extent.width = dimensions;
            imageCreateInfo.extent.height = dimensions;
            imageCreateInfo.extent.depth = 1;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VK_CHECK(vkCreateImage(m_device.lDevice(), &imageCreateInfo, nullptr, &offScreen.image));
            

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(m_device.lDevice(), offScreen.image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = utils::findMemoryType(m_device.pDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VK_CHECK(vkAllocateMemory(m_device.lDevice(), &allocInfo, nullptr, &offScreen.memory));
            VK_CHECK(vkBindImageMemory(m_device.lDevice(), offScreen.image, offScreen.memory, 0));
            
            VkImageViewCreateInfo colorImageView{};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = format;
            colorImageView.flags = 0;
            colorImageView.subresourceRange = {};
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;
            colorImageView.image = offScreen.image;
            VK_CHECK(vkCreateImageView(m_device.lDevice(), &colorImageView, nullptr, &offScreen.view));
            
            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = renderPass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &offScreen.view;
            fbufCreateInfo.width = dimensions;
            fbufCreateInfo.height = dimensions;
            fbufCreateInfo.layers = 1;
            VK_CHECK(vkCreateFramebuffer(m_device.lDevice(), &fbufCreateInfo, nullptr, &offScreen.frameBuffer));

            VkCommandBuffer cmdBuf;
            commandBuffer.allocCommandBuffer(cmdBuf);
            commandBuffer.beginCommand(cmdBuf);
            
            // vks::tools::setImageLayout(
            //         layoutCmd,
            //         offscreen.image,
            //         VK_IMAGE_ASPECT_COLOR_BIT,
            //         VK_IMAGE_LAYOUT_UNDEFINED,
            //         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            // vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);
        }
    }

    void CubeMapSystem::generatePreFilteredCube()
    {
    }
} // karhu namespace
