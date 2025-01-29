#pragma once
#include "kDevice.hpp"
#include "types.hpp"

namespace karhu
{
	class kModel
	{
	public:
		kModel(Vulkan_Device& device, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices, VkCommandPool commandPool);
		~kModel();

		kModel(const kModel&) = delete;
		void operator=(const kModel&) = delete;

		struct {
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
		}m_VertexBuffer;

		struct {
			VkBuffer m_IndexBuffer;
			VkDeviceMemory m_IndexBufferMemory;
		}m_IndexBuffer;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer, std::vector<uint16_t> indices);
	private:
		void createVertexBuffer(const std::vector<Vertex> vertices, VkCommandPool commandPool);
		void createIndexBuffer(const std::vector<uint16_t> indices, VkCommandPool commandPool);
	private:
		Vulkan_Device& m_Device;
	};
}