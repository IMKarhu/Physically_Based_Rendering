#pragma once
#include "kDevice.hpp"
#include "types.hpp"

#include <assimp/scene.h>

namespace karhu
{
	class kModel
	{
	public:
		kModel(Vulkan_Device& device, std::string filepath, VkCommandPool commandPool);
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
		void draw(VkCommandBuffer commandBuffer, std::vector<uint32_t> indices);
		void loadModel(std::string filepath);
		void processScene(const aiScene* scene);
		void processNode(aiMesh* mesh);

		std::vector<uint32_t>& getIndices() { return m_Indices; }
	private:
		void createVertexBuffer(const std::vector<Vertex> vertices, VkCommandPool commandPool);
		void createIndexBuffer(const std::vector<uint32_t> indices, VkCommandPool commandPool);
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Vulkan_Device& m_Device;
	};
}