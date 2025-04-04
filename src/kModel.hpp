#pragma once
#include "kDevice.hpp"
#include "types.hpp"
#include "kTexture.hpp"

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

		struct {
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_Texture;

		struct {
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_NormalTexture;

		struct {
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_MetallicTexture;

		struct {
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_RoughnessTexture;

		struct {
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_AoTexture;

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
	};
}