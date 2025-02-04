#include "kModel.hpp"
#include <assimp/Importer.hpp>

#include <assimp/postprocess.h>

namespace karhu
{
	kModel::kModel(Vulkan_Device& device, std::string filepath, VkCommandPool commandPool)
		:m_Device(device)
	{
		loadModel(filepath);
		createVertexBuffer(m_Vertices, commandPool);
		createIndexBuffer(m_Indices, commandPool);
	}

	kModel::~kModel()
	{
		vkDestroyBuffer(m_Device.m_Device, m_IndexBuffer.m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_IndexBuffer.m_IndexBufferMemory, nullptr);
		vkDestroyBuffer(m_Device.m_Device, m_VertexBuffer.m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_VertexBuffer.m_VertexBufferMemory, nullptr);
		printf("model Destroyed\n");
	}

	void kModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vBuffers[] = { m_VertexBuffer.m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void kModel::draw(VkCommandBuffer commandBuffer, std::vector<uint32_t> indices)
	{
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	void kModel::loadModel(std::string filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (scene == nullptr)
		{
			std::cout << "Unable to read file! error message: \n" << std::endl;
			throw std::runtime_error(importer.GetErrorString());
		}
		processScene(scene);
	}

	void kModel::processScene(const aiScene* scene)
	{
		aiNode* node = scene->mRootNode;
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			processNode(mesh);
		}
	}

	void kModel::processNode(aiMesh* mesh)
	{
		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vert;

			vert.pos.x = mesh->mVertices[i].x;
			vert.pos.y = mesh->mVertices[i].y;
			vert.pos.z = mesh->mVertices[i].z;

			m_Vertices.push_back(vert);
		}

		for (int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (int j = 0; j < face.mNumIndices; j++)
			{
				m_Indices.push_back(face.mIndices[j]);
			}
		}
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

	void kModel::createIndexBuffer(const std::vector<uint32_t> indices, VkCommandPool commandPool)
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