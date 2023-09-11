#pragma once
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <array>
#include <vector>

namespace kge
{
	class vkDevice;

	class kgeModel
	{
	public:
		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
			//VkDescriptorSetLayoutBinding uboBinding{};
		};
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;

			static VkVertexInputBindingDescription getbindingDescriptions();
			static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
		};

		kgeModel(vkDevice* device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices, uint32_t maxFrames);
		kgeModel(const kgeModel&) = delete; /* Disallows copying. */
		~kgeModel();
		kgeModel& operator=(const kgeModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		void updateUniformBuffer(uint32_t currentImage, uint32_t width, uint32_t height);

	private:
		vkDevice* m_Device;

		/* Vertex variables. */
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		const std::vector<Vertex> vertices;
		uint32_t m_VerticeCount;

		/* Index variables. */
		VkBuffer m_IndeBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		const std::vector<uint16_t> indices;
		uint32_t m_IndiceCount;

		/* Uniform Variables. */
		VkBuffer m_UniformBuffer;
		VkDeviceMemory m_UniformBufferMemory;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformbufferMemories;
		std::vector<void*> m_UniformBuffersMapped;

		void createVertexBuffer(const std::vector<Vertex> &vertices);
		void createIndexBuffer(const std::vector<uint16_t>& indices);
		void createUniformBuffers(uint32_t maxFrames);
		
	};
}