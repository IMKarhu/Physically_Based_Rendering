#include "kDescriptors.hpp"
#include "kDevice.hpp"
#include "kModel.hpp"

namespace karhu
{
    kDescriptors::kDescriptors(Vulkan_Device& device)
        :m_VkDevice(device)
    {
    }

    kDescriptors::~kDescriptors()
    {
        vkDestroyDescriptorPool(m_VkDevice.m_Device, m_DescriptorPool, nullptr);
    }

    VkDescriptorSetLayout kDescriptors::createDescriptorSetLayout(VkDescriptorSetLayout layout)
    {
        std::vector< VkDescriptorSetLayoutBinding> bindings;
        for (auto bind : m_Bindings)
        {
            bindings.push_back(bind.second);
        }
        VkDescriptorSetLayoutCreateInfo layoutCI{};
        layoutCI = Helpers::fillDescriptorSetLayoutCreateInfo();
        layoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutCI.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_VkDevice.m_Device, &layoutCI, nullptr, &layout));
        return layout;
    }
    void kDescriptors::addBind(uint32_t binding, VkDescriptorType type, uint32_t descriptorcount, VkShaderStageFlagBits stageflags)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = type;
        layoutBinding.descriptorCount = descriptorcount;
        layoutBinding.stageFlags = stageflags;
        layoutBinding.pImmutableSamplers = nullptr;
        //layoutBinding = Helpers::fillLayoutBindingStruct(binding, type, descriptorcount, stageflags);
        m_Bindings[binding] = layoutBinding;
    }
    
    void kDescriptors::createDescriptorPool(const int maxFramesInFlight)
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolcreateInfo.pPoolSizes = poolSizes.data();
        poolcreateInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

        VK_CHECK(vkCreateDescriptorPool(m_VkDevice.m_Device, &poolcreateInfo, nullptr, &m_DescriptorPool));
    }
    
    std::vector<VkDescriptorSet> kDescriptors::createDescriptorSets(const int maxFramesInFlight, VkDescriptorSetLayout layout, const std::vector<VkBuffer>& uniformBuffers, const Texture& texture)
    {
        std::vector<VkDescriptorSet> descriptorSets{};
        createDescriptorPool(maxFramesInFlight);
        std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(maxFramesInFlight);
        VK_CHECK(vkAllocateDescriptorSets(m_VkDevice.m_Device, &allocInfo, descriptorSets.data()));

        for (size_t i = 0; i < maxFramesInFlight; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.imageView();
            imageInfo.sampler = texture.textureSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_VkDevice.m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        return descriptorSets;
    }
    /*
    void kDescriptors::cleanUp(VkDevice device)
    {

    }*/


}