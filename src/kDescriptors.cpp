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

    VkDescriptorSetLayout kDescriptors::createDescriptorSetLayout(std::vector< VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayout layout{};
        /*VkDescriptorSetLayoutBinding binding{};
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
        aoSamplerBinding = Helpers::fillLayoutBindingStruct(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);*/

        /*std::array<VkDescriptorSetLayoutBinding, 6> bindings = { binding, samplerBinding, normalSamplerBinding, metallicSamplerBinding, roughnessSamplerBinding, aoSamplerBinding };*/
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
        /*if (m_WritesData.size() == 6)
        {
            m_Writes.emplace(bindingID, m_WritesData);
            m_WritesData.clear();
        }*/
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
        //m_Writes.emplace(bindingID, m_WritesData);
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


    DescriptorBuilder::DescriptorBuilder(VkDescriptorPool& pool, VkDescriptorSetLayout& layout)
        : m_Pool(pool)
        , m_layout(layout)
    {
    }

}