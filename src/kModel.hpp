#pragma once
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "types.hpp"
#include "kTexture.hpp"

#include <assimp/scene.h>

namespace karhu
{

	class kModel
	{
	public:
		kModel(Vulkan_Device& device, Vulkan_SwapChain& swapChain, std::string filepath, VkCommandPool commandPool);
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
		void draw(VkCommandBuffer commandBuffer);
		void loadModel(std::string filepath);
		void processScene(const aiScene* scene);
		void processNode(aiMesh* mesh, const aiScene* scene);

		std::vector<uint32_t>& getIndices() { return m_Indices; }
		void createVertexBuffer(VkCommandPool commandPool);
		void createIndexBuffer(VkCommandPool commandPool);
		std::vector<kTexture> m_Textures;
	private:
		
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
	};
}