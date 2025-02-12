#include "kDescriptors.hpp"
#include "kEntity.hpp"

namespace karhu
{
    kDescriptors::kDescriptors(Vulkan_Device& device)
        :m_Device(device)
    {
    }

    kDescriptors::~kDescriptors()
    {
        vkDestroyDescriptorPool(m_Device.m_Device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_Device.m_Device, m_DescriptorLayout, nullptr);
    }

    void kDescriptors::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding binding{};
        binding = Helpers::fillLayoutBindingStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding = Helpers::fillLayoutBindingStruct(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayoutBinding normalSamplerBinding{};
        normalSamplerBinding = Helpers::fillLayoutBindingStruct(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayoutBinding metallicSamplerBinding{};
        metallicSamplerBinding = Helpers::fillLayoutBindingStruct(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayoutBinding roughnessSamplerBinding{};
        roughnessSamplerBinding = Helpers::fillLayoutBindingStruct(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayoutBinding aoSamplerBinding{};
        aoSamplerBinding = Helpers::fillLayoutBindingStruct(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        std::array<VkDescriptorSetLayoutBinding, 6> bindings = { binding, samplerBinding, normalSamplerBinding, metallicSamplerBinding, roughnessSamplerBinding, aoSamplerBinding };
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_Device.m_Device, &createInfo, nullptr, &m_DescriptorLayout));
    }

    void kDescriptors::createDescriptorPool(uint32_t entityCount)
    {
        std::array<VkDescriptorPoolSize, 2> poolSize{};
        poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize[0].descriptorCount = entityCount; //count of objects?
        poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize[1].descriptorCount = entityCount;

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
        poolcreateInfo.pPoolSizes = poolSize.data();
        poolcreateInfo.maxSets = entityCount; //count of objects?

        VK_CHECK(vkCreateDescriptorPool(m_Device.m_Device, &poolcreateInfo, nullptr, &m_DescriptorPool));
    }

    void kDescriptors::createDescriptorSets(std::vector<kEntity>& entities)
    {
        
        /* loop through every object and create a descriptor set*/
        for (auto& entity : entities)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_DescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_DescriptorLayout;
            printf("hello3\n");
            VK_CHECK(vkAllocateDescriptorSets(m_Device.m_Device, &allocInfo, &entity.m_DescriptorSet));

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = entity.m_UniformBuffer.m_Buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = entity.getModel()->m_Textures[0].m_TextureVars.m_TextureView;
            imageInfo.sampler = entity.getModel()->m_Textures[0].m_TextureVars.m_Sampler;

            VkDescriptorImageInfo NormalImageInfo{};
            NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            NormalImageInfo.imageView = entity.getModel()->m_Textures[1].m_TextureVars.m_TextureView;
            NormalImageInfo.sampler = entity.getModel()->m_Textures[1].m_TextureVars.m_Sampler;

            VkDescriptorImageInfo MetallicImageInfo{};
            MetallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            MetallicImageInfo.imageView = entity.getModel()->m_Textures[2].m_TextureVars.m_TextureView;
            MetallicImageInfo.sampler = entity.getModel()->m_Textures[2].m_TextureVars.m_Sampler;

            VkDescriptorImageInfo RoughnessImageInfo{};
            RoughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            RoughnessImageInfo.imageView = entity.getModel()->m_Textures[3].m_TextureVars.m_TextureView;
            RoughnessImageInfo.sampler = entity.getModel()->m_Textures[3].m_TextureVars.m_Sampler;

            VkDescriptorImageInfo AoImageInfo{};
            AoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            AoImageInfo.imageView = entity.getModel()->m_Textures[4].m_TextureVars.m_TextureView;
            AoImageInfo.sampler = entity.getModel()->m_Textures[4].m_TextureVars.m_Sampler;

            std::array<VkWriteDescriptorSet, 6> descriptorWrite{};
            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = entity.m_DescriptorSet;
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;

            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = entity.m_DescriptorSet;
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].pImageInfo = &imageInfo;

            descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[2].dstSet = entity.m_DescriptorSet;
            descriptorWrite[2].dstBinding = 2;
            descriptorWrite[2].dstArrayElement = 0;
            descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[2].descriptorCount = 1;
            descriptorWrite[2].pImageInfo = &NormalImageInfo;

            descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[3].dstSet = entity.m_DescriptorSet;
            descriptorWrite[3].dstBinding = 3;
            descriptorWrite[3].dstArrayElement = 0;
            descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[3].descriptorCount = 1;
            descriptorWrite[3].pImageInfo = &MetallicImageInfo;

            descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[4].dstSet = entity.m_DescriptorSet;
            descriptorWrite[4].dstBinding = 4;
            descriptorWrite[4].dstArrayElement = 0;
            descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[4].descriptorCount = 1;
            descriptorWrite[4].pImageInfo = &RoughnessImageInfo;

            descriptorWrite[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[5].dstSet = entity.m_DescriptorSet;
            descriptorWrite[5].dstBinding = 5;
            descriptorWrite[5].dstArrayElement = 0;
            descriptorWrite[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[5].descriptorCount = 1;
            descriptorWrite[5].pImageInfo = &AoImageInfo;


            vkUpdateDescriptorSets(m_Device.m_Device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
        }
    }

    void kDescriptors::cleanUp(VkDevice device)
    {

    }


}