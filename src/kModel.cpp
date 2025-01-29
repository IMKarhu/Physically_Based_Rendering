#include "kModel.hpp"

namespace karhu
{
	kModel::kModel(Vulkan_Device& device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices, VkCommandPool commandPool)
		:m_Device(device)
	{
		createVertexBuffer(vertices, commandPool);
		createIndexBuffer(indices, commandPool);
	}

	kModel::~kModel()
	{
		vkDestroyBuffer(m_Device.m_Device, m_IndexBuffer.m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_IndexBuffer.m_IndexBufferMemory, nullptr);
		vkDestroyBuffer(m_Device.m_Device, m_VertexBuffer.m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_VertexBuffer.m_VertexBufferMemory, nullptr);
	}

	void kModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vBuffers[] = { m_VertexBuffer.m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void kModel::draw(VkCommandBuffer commandBuffer, std::vector<uint16_t> indices)
	{
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	void kModel::createVertexBuffer(const std::vector<Vertex> vertices, VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		VK_CHECK(vkMapMemory(m_Device.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.m_Device, stagingBufferMemory);

		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer.m_VertexBuffer, m_VertexBuffer.m_VertexBufferMemory);

		m_Device.copyBuffers(stagingBuffer, m_VertexBuffer.m_VertexBuffer, bufferSize, commandPool);

		vkDestroyBuffer(m_Device.m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, stagingBufferMemory, nullptr);
	}

	void kModel::createIndexBuffer(const std::vector<uint16_t> indices, VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		VK_CHECK(vkMapMemory(m_Device.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.m_Device, stagingBufferMemory);

		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndexBuffer.m_IndexBuffer, m_IndexBuffer.m_IndexBufferMemory);

		m_Device.copyBuffers(stagingBuffer, m_IndexBuffer.m_IndexBuffer, bufferSize, commandPool);

		vkDestroyBuffer(m_Device.m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, stagingBufferMemory, nullptr);
	}
}