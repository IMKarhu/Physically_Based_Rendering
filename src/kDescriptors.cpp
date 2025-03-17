#include "kDescriptors.hpp"
#include "kEntity.hpp"

namespace karhu
{
    //kDescriptors::kDescriptors(Vulkan_Device& device)
    //    :m_Device(device)
    //{
    //}

    //kDescriptors::~kDescriptors()
    //{
    //    vkDestroyDescriptorPool(m_Device.m_Device, m_DescriptorPool, nullptr);
    //    //vkDestroyDescriptorSetLayout(m_Device.m_Device, m_DescriptorLayout, nullptr);
    //}

    //VkDescriptorSetLayout kDescriptors::createDescriptorSetLayout()
    //{
    //    VkDescriptorSetLayout setLayout;
    //   /* VkDescriptorSetLayoutBinding binding{};
    //    binding = Helpers::fillLayoutBindingStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

    //    VkDescriptorSetLayoutBinding samplerBinding{};
    //    samplerBinding = Helpers::fillLayoutBindingStruct(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    //    VkDescriptorSetLayoutBinding normalSamplerBinding{};
    //    normalSamplerBinding = Helpers::fillLayoutBindingStruct(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    //    VkDescriptorSetLayoutBinding metallicSamplerBinding{};
    //    metallicSamplerBinding = Helpers::fillLayoutBindingStruct(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    //    VkDescriptorSetLayoutBinding roughnessSamplerBinding{};
    //    roughnessSamplerBinding = Helpers::fillLayoutBindingStruct(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    //    VkDescriptorSetLayoutBinding aoSamplerBinding{};
    //    aoSamplerBinding = Helpers::fillLayoutBindingStruct(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);*/

    //    //std::array<VkDescriptorSetLayoutBinding, 6> bindings = { binding, samplerBinding, normalSamplerBinding, metallicSamplerBinding, roughnessSamplerBinding, aoSamplerBinding };
    //    VkDescriptorSetLayoutCreateInfo createInfo{};
    //    createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
    //    createInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
    //    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    //    createInfo.pBindings = m_Bindings.data();

    //    VK_CHECK(vkCreateDescriptorSetLayout(m_Device.m_Device, &createInfo, nullptr, &setLayout));

    //    return setLayout;
    //}

    //VkDescriptorPool kDescriptors::createDescriptorPool(uint32_t maxSets)
    //{
    //    VkDescriptorPool pool;
    //    /*
    //    std::array<VkDescriptorPoolSize, 2> poolSize{};
    //    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //    poolSize[0].descriptorCount = entityCount; //count of objects?
    //    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //    poolSize[1].descriptorCount = entityCount;*/

    //    VkDescriptorPoolCreateInfo poolcreateInfo{};
    //    poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //    poolcreateInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
    //    poolcreateInfo.pPoolSizes = m_PoolSizes.data();
    //    poolcreateInfo.maxSets = maxSets;

    //    VK_CHECK(vkCreateDescriptorPool(m_Device.m_Device, &poolcreateInfo, nullptr, &pool));

    //    return pool;
    //}

    //void kDescriptors::updateDescriptorSet()
    //{
    //    vkUpdateDescriptorSets(m_Device.m_Device, static_cast<uint32_t>(m_SetWrites.size()), m_SetWrites.data(), 0, nullptr);
    //    /* loop through every object and create a descriptor set*/
    //        /*VkDescriptorSetAllocateInfo allocInfo{};
    //        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //        allocInfo.descriptorPool = pool;
    //        allocInfo.descriptorSetCount = 1;
    //        allocInfo.pSetLayouts = &layout;
    //        printf("hello3\n");
    //        VK_CHECK(vkAllocateDescriptorSets(m_Device.m_Device, &allocInfo, &set));*/

    //        /*VkDescriptorBufferInfo bufferInfo{};
    //        bufferInfo.buffer = buffer.m_Buffer;
    //        bufferInfo.offset = 0;
    //        bufferInfo.range = sizeof(UniformBufferObject);*/

    //       /* VkDescriptorImageInfo imageInfo{};
    //        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        imageInfo.imageView = entity.getModel()->m_Textures[0].m_TextureVars.m_TextureView;
    //        imageInfo.sampler = entity.getModel()->m_Textures[0].m_TextureVars.m_Sampler;

    //        VkDescriptorImageInfo NormalImageInfo{};
    //        NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        NormalImageInfo.imageView = entity.getModel()->m_Textures[1].m_TextureVars.m_TextureView;
    //        NormalImageInfo.sampler = entity.getModel()->m_Textures[1].m_TextureVars.m_Sampler;

    //        VkDescriptorImageInfo MetallicImageInfo{};
    //        MetallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        MetallicImageInfo.imageView = entity.getModel()->m_Textures[2].m_TextureVars.m_TextureView;
    //        MetallicImageInfo.sampler = entity.getModel()->m_Textures[2].m_TextureVars.m_Sampler;

    //        VkDescriptorImageInfo RoughnessImageInfo{};
    //        RoughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        RoughnessImageInfo.imageView = entity.getModel()->m_Textures[3].m_TextureVars.m_TextureView;
    //        RoughnessImageInfo.sampler = entity.getModel()->m_Textures[3].m_TextureVars.m_Sampler;

    //        VkDescriptorImageInfo AoImageInfo{};
    //        AoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        AoImageInfo.imageView = entity.getModel()->m_Textures[4].m_TextureVars.m_TextureView;
    //        AoImageInfo.sampler = entity.getModel()->m_Textures[4].m_TextureVars.m_Sampler;

    //        std::array<VkWriteDescriptorSet, 6> descriptorWrite{};
    //        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[0].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[0].dstBinding = 0;
    //        descriptorWrite[0].dstArrayElement = 0;
    //        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //        descriptorWrite[0].descriptorCount = 1;
    //        descriptorWrite[0].pBufferInfo = &bufferInfo;

    //        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[1].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[1].dstBinding = 1;
    //        descriptorWrite[1].dstArrayElement = 0;
    //        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite[1].descriptorCount = 1;
    //        descriptorWrite[1].pImageInfo = &imageInfo;

    //        descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[2].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[2].dstBinding = 2;
    //        descriptorWrite[2].dstArrayElement = 0;
    //        descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite[2].descriptorCount = 1;
    //        descriptorWrite[2].pImageInfo = &NormalImageInfo;

    //        descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[3].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[3].dstBinding = 3;
    //        descriptorWrite[3].dstArrayElement = 0;
    //        descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite[3].descriptorCount = 1;
    //        descriptorWrite[3].pImageInfo = &MetallicImageInfo;

    //        descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[4].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[4].dstBinding = 4;
    //        descriptorWrite[4].dstArrayElement = 0;
    //        descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite[4].descriptorCount = 1;
    //        descriptorWrite[4].pImageInfo = &RoughnessImageInfo;

    //        descriptorWrite[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite[5].dstSet = entity.m_DescriptorSet;
    //        descriptorWrite[5].dstBinding = 5;
    //        descriptorWrite[5].dstArrayElement = 0;
    //        descriptorWrite[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite[5].descriptorCount = 1;
    //        descriptorWrite[5].pImageInfo = &AoImageInfo;*/


    //        
    //}

    //void kDescriptors::addBinding(uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags)
    //{
    //    VkDescriptorSetLayoutBinding tempBinding{};
    //    tempBinding = Helpers::fillLayoutBindingStruct(binding, type, descriptorCount, flags);

    //    m_Bindings.push_back(tempBinding);
    //}

    //void kDescriptors::addPool(VkDescriptorType type, uint32_t count)
    //{
    //    VkDescriptorPoolSize pool;
    //    pool.type = type;
    //    pool.descriptorCount = count;

    //    m_PoolSizes.push_back(pool);
    //}

    //void kDescriptors::allocateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& set)
    //{
    //    VkDescriptorSetAllocateInfo allocInfo{};
    //    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //    allocInfo.descriptorPool = pool;
    //    allocInfo.descriptorSetCount = 2;
    //    allocInfo.pSetLayouts = &layout;
    //    printf("hello3\n");
    //    VK_CHECK(vkAllocateDescriptorSets(m_Device.m_Device, &allocInfo, &set));
    //}

    //void kDescriptors::prepareBuffer(VkPhysicalDeviceDescriptorBufferPropertiesEXT properties)
    //{
    //    VkPhysicalDeviceDescriptorBufferPropertiesEXT dsBufferProperties{};
    //    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

    //    VkPhysicalDeviceProperties2KHR deviceProperties{};
    //    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    //    deviceProperties.pNext = &properties;
    //    vkGetPhysicalDeviceProperties2KHR(m_Device.m_PhysicalDevice, &deviceProperties);
    //}

    //void kDescriptors::write(kBuffer& buffer, VkDescriptorSet& set)
    //{
    //    VkDescriptorBufferInfo bufferInfo{};
    //    bufferInfo.buffer = buffer.m_Buffer;
    //    bufferInfo.offset = 0;
    //    bufferInfo.range = sizeof(UniformBufferObject);

    //    VkWriteDescriptorSet descriptorWrite{};
    //    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    descriptorWrite.dstSet = set;
    //    descriptorWrite.dstBinding = 0;
    //    descriptorWrite.dstArrayElement = 0;
    //    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //    descriptorWrite.descriptorCount = 1;
    //    descriptorWrite.pBufferInfo = &bufferInfo;

    //    m_SetWrites.push_back(descriptorWrite);
    //    
    //}

    //void kDescriptors::writeImg(kEntity& entity)
    //{
    //    for (size_t i = 0; i < entity.getModel()->m_Textures.size(); i++)
    //    {
    //        VkDescriptorImageInfo imageInfo{};
    //        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        imageInfo.imageView = entity.getModel()->m_Textures[i].m_TextureVars.m_TextureView;
    //        imageInfo.sampler = entity.getModel()->m_Textures[i].m_TextureVars.m_Sampler;

    //        VkWriteDescriptorSet descriptorWrite{};
    //        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //        descriptorWrite.dstSet = entity.m_DescriptorSet;
    //        descriptorWrite.dstBinding = i+1;
    //        descriptorWrite.dstArrayElement = 0;
    //        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //        descriptorWrite.descriptorCount = 1;
    //        descriptorWrite.pImageInfo = &imageInfo;

    //        m_SetWrites.push_back(descriptorWrite);
    //    }
    //    
    //}

    //void kDescriptors::freeBindings()
    //{
    //    m_Bindings.clear();
    //}

    //void kDescriptors::cleanUp(VkDevice device)
    //{

    //}

// *************** Descriptor Set Layout Builder *********************

LveDescriptorSetLayout::Builder& LveDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<LveDescriptorSetLayout> LveDescriptorSetLayout::Builder::build() const {
    return std::make_unique<LveDescriptorSetLayout>(m_Device, bindings);
}

// *************** Descriptor Set Layout *********************

LveDescriptorSetLayout::LveDescriptorSetLayout(
    Vulkan_Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m_Device{ device }, bindings{ bindings } {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(
        m_Device.m_Device,
        &descriptorSetLayoutInfo,
        nullptr,
        &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

LveDescriptorSetLayout::~LveDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(m_Device.m_Device, descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

LveDescriptorPool::Builder& LveDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({ descriptorType, count });
    return *this;
}

LveDescriptorPool::Builder& LveDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}
LveDescriptorPool::Builder& LveDescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}

std::unique_ptr<LveDescriptorPool> LveDescriptorPool::Builder::build() const {
    return std::make_unique<LveDescriptorPool>(m_Device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

LveDescriptorPool::LveDescriptorPool(
    Vulkan_Device& lveDevice,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize>& poolSizes)
    : m_Device{ lveDevice } {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(m_Device.m_Device, &descriptorPoolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

LveDescriptorPool::~LveDescriptorPool() {
    vkDestroyDescriptorPool(m_Device.m_Device, descriptorPool, nullptr);
}

bool LveDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(m_Device.m_Device, &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void LveDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
    vkFreeDescriptorSets(
        m_Device.m_Device,
        descriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void LveDescriptorPool::resetPool() {
    vkResetDescriptorPool(m_Device.m_Device, descriptorPool, 0);
}

// *************** Descriptor Writer *********************

LveDescriptorWriter::LveDescriptorWriter(LveDescriptorSetLayout& setLayout, LveDescriptorPool& pool)
    : setLayout{ setLayout }, pool{ pool } {
}

LveDescriptorWriter& LveDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

LveDescriptorWriter& LveDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo* imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto& bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;
    write.dstArrayElement = 0;

    writes.push_back(write);
    return *this;
}



bool LveDescriptorWriter::build(VkDescriptorSet& set) {
    bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
    if (!success) {
        printf("failed allocate descriptor\n");
        return false;
    }
    overwrite(set);
    return true;
}

void LveDescriptorWriter::overwrite(VkDescriptorSet& set) {
    for (auto& write : writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool.m_Device.m_Device, writes.size(), writes.data(), 0, nullptr);
}


}