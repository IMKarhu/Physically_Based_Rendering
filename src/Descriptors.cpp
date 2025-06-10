#include "Descriptors.hpp"
#include "Entity.hpp"
#include "utils/utils.hpp"

namespace karhu
{
    Descriptors::Descriptors(Device& device)
        :m_device(device)
    {
    }

    Descriptors::~Descriptors()
    {
    }

    VkDescriptorSetLayout Descriptors::createDescriptorSetLayout(std::vector< VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayout layout{};
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = utils::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_device.lDevice(), &createInfo, nullptr, &layout));

        return layout;
    }

    void Descriptors::bind(std::vector< VkDescriptorSetLayoutBinding>& bindings, uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags)
    {
        VkDescriptorSetLayoutBinding bind{};
        bind = utils::fillLayoutBindingStruct(binding, type, descriptorCount, flags);

        bindings.push_back(bind);
    }

    VkDescriptorPool Descriptors::createDescriptorPool(uint32_t maxSets)
    {
        VkDescriptorPool pool;
        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
        poolcreateInfo.pPoolSizes = m_poolSizes.data();
        poolcreateInfo.maxSets = maxSets;

        VK_CHECK(vkCreateDescriptorPool(m_device.lDevice(), &poolcreateInfo, nullptr, &pool));

        return pool;
    }

    void Descriptors::createDescriptorSets(std::vector<Entity>& entities, VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        
        /* loop through every object and create a descriptor set*/
        for (auto& entity : entities)
        {
            vkUpdateDescriptorSets(m_device.lDevice(), static_cast<uint32_t>(m_writes[entity.getId()].size()), m_writes[entity.getId()].data(), 0, nullptr);
        }
    }

    void Descriptors::createDescriptorSets(VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        vkUpdateDescriptorSets(m_device.lDevice(), static_cast<uint32_t>(m_writes[0].size()), m_writes[0].data(), 0, nullptr);
    }

    void Descriptors::cleanUp(VkDevice device)
    {

    }

    void Descriptors::addPoolElement(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = count;

        m_poolSizes.push_back(poolSize);
    }

    void Descriptors::writeImg(VkDescriptorSet& set, uint32_t binding, VkDescriptorImageInfo info, uint32_t bindingID)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &info;

        m_writesData.push_back(descriptorWrite);

    }

    void Descriptors::writeBuffer(VkDescriptorSet& set, uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo info, uint32_t bindingID)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &info;

        m_writesData.push_back(descriptorWrite);
    }

    void Descriptors::fillWritesMap(uint32_t bindingID)
    {
        m_writes.emplace(bindingID, m_writesData);
        if (m_writesData.size() == 6)
        {
            m_writesData.clear();
        }
    }

    void Descriptors::allocateDescriptor(VkDescriptorSet& set, VkDescriptorSetLayout layout, VkDescriptorPool pool)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        printf("hello3\n");
        VK_CHECK(vkAllocateDescriptorSets(m_device.lDevice(), &allocInfo, &set));
    }
}
