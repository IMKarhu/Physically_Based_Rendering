#include "descriptor.h"
#include <array>

namespace kge
{
    Descriptor::Descriptor(Device* device)
        :m_Device(device)
    {
    }
    Descriptor::~Descriptor()
    {
        //delete m_Device;
    }
    void Descriptor::createDescriptorSetLayout()
	{
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; /* Binding number in sahder. */
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; /* Type of descriptor. */
        uboLayoutBinding.descriptorCount = 1; /* Number of values in the array. Our MVP transformation is in a single uniform buffer object. */
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; /**/
        uboLayoutBinding.pImmutableSamplers = nullptr; /* optional. Only relevant for image sampling descriptors.*/

        VkDescriptorSetLayoutBinding samplerLayoutbinding{};
        samplerLayoutbinding.binding = 1; /* Binding number in sahder. */
        samplerLayoutbinding.descriptorCount = 1; /* Number of descriptors contained in the binding. */
        samplerLayoutbinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; /* Which type of descriptor is used for binding. */
        samplerLayoutbinding.pImmutableSamplers = nullptr;
        samplerLayoutbinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; /* Specify which pipeline shader stages can access a resource for this binding. */

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutbinding };
        VkDescriptorSetLayoutCreateInfo layinfo{};
        layinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; /* Value identifying this structure. */
        layinfo.bindingCount = static_cast<uint32_t>(bindings.size()); /* Number of elements in pBindings. */
        layinfo.pBindings = bindings.data(); /* Pointer to our layoutbinding array. */

        if (vkCreateDescriptorSetLayout(m_Device->getDevice(), &layinfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
	}
	void Descriptor::createDescriptorPool(const int maxframesinflight)
	{
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(maxframesinflight);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(maxframesinflight);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(maxframesinflight);

        if (vkCreateDescriptorPool(m_Device->getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
	}
    void Descriptor::createDescriptorSets(UniformBuffers* uniformbuffers, const int maxframesinflight, const VkImageView& textureImageView, const VkSampler& textureSampler)
    {
        std::vector<VkDescriptorSetLayout> layouts(maxframesinflight, m_DescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(maxframesinflight);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(maxframesinflight);
        if (vkAllocateDescriptorSets(m_Device->getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < maxframesinflight; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformbuffers->getUniformBuffers()[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr; /* Used for descriptors that refer to image data. */
            descriptorWrites[0].pTexelBufferView = nullptr; /* Used for descriptors that refer to buffer views. */

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_Device->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
}