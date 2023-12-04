#include "model.h"
#include "vkDevice.h"

namespace kge
{
	kgeModel::kgeModel(vkDevice* device, const std::vector<Vertex> &vertices, const std::vector<uint16_t>& indices, uint32_t maxFrames)
		:m_Device(device)
	{
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
		createUniformBuffers(maxFrames);
	}

	kgeModel::~kgeModel()
	{
		vkDestroyBuffer(m_Device->getDevice(), m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device->getDevice(), m_VertexBufferMemory, nullptr);

		vkDestroyBuffer(m_Device->getDevice(), m_IndeBuffer, nullptr);
		vkFreeMemory(m_Device->getDevice(), m_IndexBufferMemory, nullptr);
		for (size_t i = 0; i < 2; i++) {
			vkDestroyBuffer(m_Device->getDevice(), m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device->getDevice(), m_UniformbufferMemories[i], nullptr);
		}
	}

	void kgeModel::bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, std::vector<VkDescriptorSet> descriptorsets, int currentframe)
	{
		VkBuffer vertexBuffers[] = { m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndeBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
			&descriptorsets[currentframe], 0, nullptr);
	}

	void kgeModel::draw(VkCommandBuffer commandBuffer)
	{
		//vkCmdDraw(commandBuffer, m_VerticeCount, 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, m_IndiceCount, 1, 0, 0, 0);
	}

	void kgeModel::updateUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		kgeModel::UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}


	void kgeModel::createVertexBuffer(const std::vector<Vertex> &vertices)
	{
		m_VerticeCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		m_Device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);

		m_Device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer, m_VertexBufferMemory);

		m_Device->copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

		vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);
	}

	void kgeModel::createIndexBuffer(const std::vector<uint16_t>& indices)
	{
		m_IndiceCount = static_cast<uint32_t>(indices.size());
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		m_Device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);

		m_Device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndeBuffer, m_IndexBufferMemory);

		m_Device->copyBuffer(stagingBuffer, m_IndeBuffer, bufferSize);

		vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);
	}

	void kgeModel::createUniformBuffers(uint32_t maxFrames)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(maxFrames);
		m_UniformbufferMemories.resize(maxFrames);
		m_UniformBuffersMapped.resize(maxFrames);

		for (size_t i = 0; i < maxFrames; i++)
		{
			m_Device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_UniformBuffers[i], m_UniformbufferMemories[i]);
			vkMapMemory(m_Device->getDevice(), m_UniformbufferMemories[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
		}
	}

	VkVertexInputBindingDescription kgeModel::Vertex::getbindingDescriptions()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	std::array<VkVertexInputAttributeDescription, 2> kgeModel::Vertex::getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
}