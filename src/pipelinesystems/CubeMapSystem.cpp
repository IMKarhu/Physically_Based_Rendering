#include "CubeMapSystem.hpp"

#include "../Device.hpp"
#include "../Entity.hpp"
#include "../Descriptors.hpp"
#include "../FrameBuffer.hpp"

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

    void CubeMapSystem::generateBrdfLut(VkRenderPass renderPass, VkFramebuffer frameBuffer)
    {
        const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        const uint32_t dimensions = 512;

        m_textures.m_brdfLut = Image(m_device.lDevice(),
                m_device.pDevice(),
                dimensions,
                dimensions,
                format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_textures.m_brdfLut.createSampler(m_device.lDevice());

        karhu::createFrameBuffer1(m_device.lDevice(),
                frameBuffer,
                m_textures.m_brdfLut.getImageView(),
                renderPass,
                dimensions,
                dimensions,
                1);

        VkDescriptorSetLayout layout;
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        layout = m_brdflutBuilder->createDescriptorSetLayout(bindings);
        VkDescriptorPool pool;
        m_brdflutBuilder = std::make_unique<Descriptors>(m_device);
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

        m_pipelineBuilder.createPipeline(pipelineStruct, "../shaders/brdflutvert.spv", "../shaders/brdflutfrag.spv");






    // void createFrameBuffer(VkDevice device,
    //         std::vector<VkFramebuffer>& frameBuffers,
    //         const std::vector<VkImageView>& imageViews,
    //         const VkRenderPass& renderPass,
    //         uint32_t width,
    //         uint32_t height,
    //         uint32_t layers,
    //         bool isCube = false,
    //         const VkImageView& depthView = nullptr);


        // //image creation.
        // VkImageCreateInfo createInfo{};
        // createInfo.imageType = VK_IMAGE_TYPE_2D;
        // createInfo.format = format;
        // createInfo.extent.width = 512;
        // createInfo.extent.height = 512;
        // createInfo.extent.depth = 1;
        // createInfo.mipLevels = 1;
        // createInfo.arrayLayers = 1;
        // createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        // createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // VK_CHECK(vkCreateImage(m_device.lDevice(), &createInfo, nullptr, &m_textures.brdflut));
        // VkMemoryRequirements memRequirements;
        // vkGetImageMemoryRequirements(m_device.lDevice(), m_textures.brdflut, &memRequirements);
        // VkMemoryAllocateInfo allocInfo{};
        // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        // allocInfo.allocationSize = memRequirements.size;
        // allocInfo.memoryTypeIndex = utils::findMemoryType(m_device.pDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //
        // //allocate and bind memory
        // if (vkAllocateMemory(m_device.lDevice(), &allocInfo, nullptr, &m_textures.brdfMemory) != VK_SUCCESS) {
        //     throw std::runtime_error("failed to allocate image memory!");
        // }
        // VK_CHECK(vkBindImageMemory(m_device.lDevice(), m_textures.brdflut, m_textures.brdfMemory, 0));
        //
        //
        // //imageView
        // VkImageViewCreateInfo viewCI{};
        // viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // viewCI.format = format;
        // viewCI.subresourceRange = {};
        // viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // viewCI.subresourceRange.levelCount = 1;
        // viewCI.subresourceRange.layerCount = 1;
        // viewCI.image = m_textures.brdflut;
        // VK_CHECK(vkCreateImageView(m_device.lDevice(), &viewCI, nullptr, &m_textures.brdflutView));

        //sampler
        // VkSamplerCreateInfo samplerCI{};
        // samplerCI.magFilter = VK_FILTER_LINEAR;
        // samplerCI.minFilter = VK_FILTER_LINEAR;
        // samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        // samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        // samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        // samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        // samplerCI.minLod = 0.0f;
        // samplerCI.maxLod = 1.0f;
        // samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        // VK_CHECK(vkCreateSampler(m_device.lDevice(), &samplerCI, nullptr, &m_textures.brdflutSampler));

    }

    void CubeMapSystem::generateIrradianceCube()
    {
    }

    void CubeMapSystem::generatePreFilteredCube()
    {
    }
} // karhu namespace
