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
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = 1;
        createInfo.pBindings = &binding;

        VK_CHECK(vkCreateDescriptorSetLayout(m_Device.m_Device, &createInfo, nullptr, &m_DescriptorLayout));
    }

    void kDescriptors::createDescriptorPool(uint32_t entityCount)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = entityCount; //count of objects?

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = 1;
        poolcreateInfo.pPoolSizes = &poolSize;
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

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = entity.m_DescriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_Device.m_Device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void kDescriptors::cleanUp(VkDevice device)
    {

    }


}