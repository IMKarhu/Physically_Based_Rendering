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
    }

    VkDescriptorSetLayout kDescriptors::createDescriptorSetLayout(std::vector< VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayout layout{};
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_Device.m_Device, &createInfo, nullptr, &layout));

        return layout;
    }

    void kDescriptors::bind(std::vector< VkDescriptorSetLayoutBinding>& bindings, uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags)
    {
        VkDescriptorSetLayoutBinding bind{};
        bind = Helpers::fillLayoutBindingStruct(binding, type, descriptorCount, flags);

        bindings.push_back(bind);
    }

    VkDescriptorPool kDescriptors::createDescriptorPool(uint32_t maxSets)
    {
        VkDescriptorPool pool;
        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
        poolcreateInfo.pPoolSizes = m_PoolSizes.data();
        poolcreateInfo.maxSets = maxSets;

        VK_CHECK(vkCreateDescriptorPool(m_Device.m_Device, &poolcreateInfo, nullptr, &pool));

        return pool;
    }

    void kDescriptors::createDescriptorSets(std::vector<kEntity>& entities, VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        
        /* loop through every object and create a descriptor set*/
        for (auto& entity : entities)
        {
            vkUpdateDescriptorSets(m_Device.m_Device, static_cast<uint32_t>(m_Writes[entity.getId()].size()), m_Writes[entity.getId()].data(), 0, nullptr);
        }
    }

    void kDescriptors::createDescriptorSets(VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        vkUpdateDescriptorSets(m_Device.m_Device, static_cast<uint32_t>(m_Writes[0].size()), m_Writes[0].data(), 0, nullptr);
    }

    void kDescriptors::cleanUp(VkDevice device)
    {

    }

    void kDescriptors::addPoolElement(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = count;

        m_PoolSizes.push_back(poolSize);
    }

    void kDescriptors::writeImg(VkDescriptorSet& set, uint32_t binding, VkDescriptorImageInfo info, uint32_t bindingID)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &info;

        m_WritesData.push_back(descriptorWrite);

    }

    void kDescriptors::writeBuffer(VkDescriptorSet& set, uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo info, uint32_t bindingID)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &info;

        m_WritesData.push_back(descriptorWrite);
    }

    void kDescriptors::fillWritesMap(uint32_t bindingID)
    {
        m_Writes.emplace(bindingID, m_WritesData);
        if (m_WritesData.size() == 6)
        {
            m_WritesData.clear();
        }
    }

    void kDescriptors::allocateDescriptor(VkDescriptorSet& set, VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        printf("hello3\n");
        VK_CHECK(vkAllocateDescriptorSets(m_Device.m_Device, &allocInfo, &set));
    }
}