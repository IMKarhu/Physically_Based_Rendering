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

        auto bindingDescription = CubeVertex::getBindingDescription();
        auto attributeDescription = CubeVertex::getAttributeDescription();

        pipelineStruct.vertexInputInfo.vertexBindingDescriptionCount = 1;
        pipelineStruct.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        pipelineStruct.vertexInputInfo.vertexAttributeDescriptionCount = 1;
        pipelineStruct.vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        /*VkPushConstantRange objPushConstant{};*/
        /*objPushConstant.offset = 0;*/
        /*objPushConstant.size = sizeof(ObjPushConstant);*/
        /*objPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;*/
        /**/
        /*VkPushConstantRange cameraPushConstant{};*/
        /*cameraPushConstant.offset = 64;*/
        /*cameraPushConstant.size = sizeof(pushConstants);*/
        /*cameraPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/
        /**/
        /*std::vector<VkPushConstantRange> pushConstantRanges{*/
        /*    objPushConstant,*/
        /*    cameraPushConstant*/
        /*};*/

        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = nullptr;


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

        pipelineStruct.vertexInputInfo.vertexBindingDescriptionCount = 0;
        pipelineStruct.vertexInputInfo.pVertexBindingDescriptions = nullptr;
        pipelineStruct.vertexInputInfo.vertexAttributeDescriptionCount = 0;
        pipelineStruct.vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
        VK_CHECK(vkEndCommandBuffer(cmdBuf));


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuf;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
        VkFence fence;
        VK_CHECK(vkCreateFence(m_device.lDevice(), &fenceInfo, nullptr, &fence));
        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitInfo, fence));
        vkWaitForFences(m_device.lDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
        commandBuffer.flushCommandBuffer(cmdBuf);

        printf("Lookup table generated!\n");
    }

    void CubeMapSystem::generateIrradianceCube(VkRenderPass renderPass,
            std::vector<VkFramebuffer>& frameBuffer,
            CommandBuffer& commandBuffer,
            Entity& entity)
    {
        printf("Start generating IrradianceCubeMap\n");
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

        karhu::createFrameBuffer1(m_device.lDevice(),
            frameBuffer,
            m_textures.m_brdfLut.getImageView(),
            renderPass,
            dimensions,
            dimensions,
            1);


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

            VkImageSubresourceRange subResourcerange{};
            subResourcerange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResourcerange.baseMipLevel = 0;
            subResourcerange.levelCount = 1;
            subResourcerange.layerCount = 1;

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.image = offScreen.image;
            barrier.subresourceRange = subResourcerange;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            vkCmdPipelineBarrier(cmdBuf,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

            commandBuffer.flushCommandBuffer(cmdBuf);
        }

        m_irradianceCubeBuilder = std::make_unique<Descriptors>(m_device);

        VkDescriptorSetLayout layout;
        std::vector<VkDescriptorSetLayoutBinding> bindings{};

        m_irradianceCubeBuilder->bind(bindings, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        layout = m_irradianceCubeBuilder->createDescriptorSetLayout(bindings);
        VkDescriptorPool pool;
        m_irradianceCubeBuilder->addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        pool = m_irradianceCubeBuilder->createDescriptorPool(2);

        VkDescriptorSet set;
        //VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
        //descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //descriptorSetAllocateInfo.descriptorPool = pool;
        //descriptorSetAllocateInfo.pSetLayouts = &layout;
        //descriptorSetAllocateInfo.descriptorSetCount = 1;
        //VK_CHECK(vkAllocateDescriptorSets(m_device.lDevice(), &descriptorSetAllocateInfo, &set));
        m_irradianceCubeBuilder->allocateDescriptor(set, layout, pool);
        m_irradianceCubeBuilder->writeImg(set, 0, entity.getModel()->m_Textures[0].getImageInfo(), 0);
        m_irradianceCubeBuilder->fillWritesMap(0);
        m_irradianceCubeBuilder->createDescriptorSets(layout, pool);

        m_irradiancePipelineBuilder.getHandle()->m_device = m_device.lDevice();

        PipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = dimensions;
        pipelineStruct.viewportheight = dimensions;
        // pipelineStruct.scissor.extent = extent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = 1;
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = &layout;
        pipelineStruct.renderPass = renderPass;

        auto bindingDescription = CubeVertex::getBindingDescription();
        auto attributeDescription = CubeVertex::getAttributeDescription();

        pipelineStruct.vertexInputInfo.vertexBindingDescriptionCount = 1;
        pipelineStruct.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        pipelineStruct.vertexInputInfo.vertexAttributeDescriptionCount = 1;
        pipelineStruct.vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        struct PushBlock {
            glm::mat4 mvp;

            float deltaPhi = (2.0 * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5 * float(M_PI)) / 64.0f;
        } pushBlock;

        VkPushConstantRange range{};
        range.size = sizeof(PushBlock);
        range.offset = 0;
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkPushConstantRange> ranges = { range };

        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = ranges.data(); // Optional


        m_irradiancePipelineBuilder.createPipeline(pipelineStruct, "../shaders/irradianceCubevert.spv", "../shaders/irradianceCubefrag.spv");

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

        VkImageSubresourceRange subResourcerange{};
        subResourcerange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subResourcerange.baseMipLevel = 0;
        subResourcerange.levelCount = mips;
        subResourcerange.layerCount = 6;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.image = m_textures.m_irradianceCube.getImage();
        barrier.subresourceRange = subResourcerange;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

        for (uint32_t i = 0; i < mips; i++)
        {
            for (uint32_t j = 0; j < 6; j++)
            {
                viewPort.width = static_cast<float>(dimensions * std::pow(0.5, i));
                viewPort.height = static_cast<float>(dimensions * std::pow(0.5, i));
                vkCmdSetViewport(cmdBuf, 0, 1, &viewPort);

                vkCmdBeginRenderPass(cmdBuf, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

                pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * m_matrices[j];
                vkCmdPushConstants(cmdBuf,
                        m_irradiancePipelineBuilder.getHandle()->m_pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        sizeof(PushBlock),
                        &pushBlock);

                m_irradiancePipelineBuilder.bind(cmdBuf);
                vkCmdBindDescriptorSets(cmdBuf,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_irradiancePipelineBuilder.getHandle()->m_pipelineLayout,
                        0,
                        1,
                        &set,
                        0,
                        NULL);

                entity.getModel()->bind(cmdBuf);
                entity.getModel()->draw(cmdBuf);

                vkCmdEndRenderPass(cmdBuf);

                VkImageSubresourceRange subResourcerange{};
                subResourcerange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subResourcerange.baseMipLevel = 0;
                subResourcerange.levelCount = 1;
                subResourcerange.layerCount = 1;

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.image = offScreen.image;
                barrier.subresourceRange = subResourcerange;
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(cmdBuf,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                // Copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion = {};
                
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };
                
                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = j;
                copyRegion.dstSubresource.mipLevel = i;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };
                
                copyRegion.extent.width = static_cast<uint32_t>(viewPort.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewPort.height);
                copyRegion.extent.depth = 1;
                
                vkCmdCopyImage(
                        cmdBuf,
                        offScreen.image,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        entity.getModel()->m_Textures[0].getImage(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &copyRegion);

                VkImageSubresourceRange subResourcerange2{};
                subResourcerange2.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subResourcerange2.baseMipLevel = 0;
                subResourcerange2.levelCount = 1;
                subResourcerange2.layerCount = 1;

                VkImageMemoryBarrier barrier2{};
                barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier2.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier2.image = offScreen.image;
                barrier2.subresourceRange = subResourcerange2;
                barrier2.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier2.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                //error here
                vkCmdPipelineBarrier(cmdBuf,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier2);
            }
        }

        VkImageSubresourceRange subResourcerange3{};
        subResourcerange3.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subResourcerange3.baseMipLevel = 0;
        subResourcerange3.levelCount = 1;
        subResourcerange3.layerCount = 1;

        VkImageMemoryBarrier barrier3{};
        barrier3.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier3.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier3.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier3.image = entity.getModel()->m_Textures[0].getImage();
        barrier3.subresourceRange = subResourcerange3;
        barrier3.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier3.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(cmdBuf,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier3);
        
        commandBuffer.flushCommandBuffer(cmdBuf);

        printf("irradianceCube generation complete!\n");
    }

    void CubeMapSystem::generatePreFilteredCube()
    {
    }
} // karhu namespace
