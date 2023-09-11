#include "buffer.h"
#include <chrono>
#include <array>

Buffer::Buffer(Device* device)
    :m_Device(device)
{
}

Buffer::~Buffer()
{
    /* Refactor for loopt to take in maxframeinflight rather than hardcoded value. */
    for (size_t i = 0; i < 2; i++) {
        vkDestroyBuffer(m_Device->getDevice(), m_UniformBuffers[i], nullptr);
        vkFreeMemory(m_Device->getDevice(), m_UniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(m_Device->getDevice(), m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device->getDevice(), m_DescriptorSetLayout, nullptr);

}

void Buffer::createDescriptorSetLayout()
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

void Buffer::createDescriptorPool(const int maxframesinflight)
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

void Buffer::createDescriptorSets(const int maxframesinflight, const VkImageView& textureImageView, const VkSampler& textureSampler)
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
        bufferInfo.buffer = m_UniformBuffers[i];
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

void Buffer::createUniformBuffers(const int maxframesinflight)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(maxframesinflight);
    m_UniformBuffersMemory.resize(maxframesinflight);
    m_UniformBuffersMapped.resize(maxframesinflight);

    for (size_t i = 0; i < maxframesinflight; i++)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_UniformBuffers[i], m_UniformBuffersMemory[i]);

        vkMapMemory(m_Device->getDevice(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
    }
}

void Buffer::updateUniformBuffers(uint32_t currentimage, const VkExtent2D& swapchainextent)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f); //glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); /* Rotate around Z axis. */
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); /* Look above at 45 degree angle. */
    /* 45 degree FOV. important to calculate aspect ratio using swapchain extent to consider window resizing. */
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchainextent.width / (float)swapchainextent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; /* a little hack to flip sign on the scaling factor of Y axis. Need to do this because glm was designed for OGL where Y coordinate of the clip is inverted.*/

    memcpy(m_UniformBuffersMapped[currentimage], &ubo, sizeof(ubo));
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequiremenets;
    vkGetBufferMemoryRequirements(m_Device->getDevice(), buffer, &memRequiremenets);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequiremenets.size;
    memAllocInfo.memoryTypeIndex = findMemoryType(memRequiremenets.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device->getDevice(), &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device->getDevice(), buffer, bufferMemory, 0);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandpool)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandpool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandpool, commandBuffer);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find memory type!");
}

VkCommandBuffer Buffer::beginSingleTimeCommands(VkCommandPool commandpool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandpool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Buffer::endSingleTimeCommands(VkCommandPool commandpool, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_Device->getGraphicsQueue());

    vkFreeCommandBuffers(m_Device->getDevice(), commandpool, 1, &commandBuffer);
}
