#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE /* By default glm uses opengl depth range of -1.0 to 1.0. We need to force it to use vulkan range of 0.0 to 1.0*/
#include <vulkan/vulkan.hpp>
#include "device.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Device;

class Buffer
{
public:
	Buffer(Device* device);
	~Buffer();

	template<typename V>
	void createVkBuffer(V& vkData, VkBuffer& buffer, VkDeviceMemory& buffermemory, VkBufferUsageFlags usage, VkDevice device, VkCommandPool commandpool)
	{
        VkDeviceSize bufferSize = sizeof(vkData[0]) * vkData.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vkData.data(), (size_t)bufferSize);
        vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            buffer, buffermemory);

        copyBuffer(stagingBuffer, buffer, bufferSize, commandpool);

        vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);
	}
	
    void createDescriptorSetLayout();
    void createDescriptorPool(const int maxframesinflight);
    void createDescriptorSets(const int maxframesinflight, const VkImageView& textureImageView, const VkSampler& textureSampler);
    void createUniformBuffers(const int maxframesinflight);
    void updateUniformBuffers(uint32_t currentimage, const VkExtent2D& swapchainextent);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); /* Refactor this into own abstarct class. */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandpool);
    void endSingleTimeCommands(VkCommandPool commandpool, VkCommandBuffer commandBuffer);


    [[nodiscard]] const VkDescriptorSetLayout getDSLayout() const { return m_DescriptorSetLayout; }
    [[nodiscard]] const std::vector<VkDescriptorSet> getDescriptorSets() const { return m_DescriptorSets; }
private:
    VkDescriptorSetLayout m_DescriptorSetLayout;
    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    std::vector<void*> m_UniformBuffersMapped;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    Device* m_Device;
    
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandpool);
    

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
};