#include "kModel.hpp"
#include <assimp/Importer.hpp>

#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

namespace karhu
{
	kModel::kModel(Vulkan_Device& device, Vulkan_SwapChain& swapChain, std::string filepath, VkCommandPool commandPool)
		:m_Device(device)
		,m_SwapChain(swapChain)
	{
		loadModel(filepath);
		std::cout << "size of vertices: " << m_Vertices.size() << std::endl;
		std::cout << "size of indices: " << m_Indices.size() << std::endl;
		createVertexBuffer(commandPool);
		createIndexBuffer(commandPool);
	}

	kModel::kModel(Vulkan_Device& device, Vulkan_SwapChain& swapChain, std::vector<Vertex> vertices, std::vector<uint32_t> indices, VkCommandPool commandPool, bool hdr)
		:m_Device(device)
		,m_SwapChain(swapChain)
	{
		createVertexBuffer(vertices, commandPool);
		createIndexBuffer(indices, commandPool);
		if (hdr)
		{
			kTexture tex{ m_Device, m_SwapChain, "monkstown_castle_4k.hdr", VK_FORMAT_R16G16B16A16_SFLOAT, true };
			m_Textures.push_back(tex);
		}
	}

	kModel::~kModel()
	{
		vkDestroyBuffer(m_Device.m_Device, m_IndexBuffer.m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_IndexBuffer.m_IndexBufferMemory, nullptr);
		vkDestroyBuffer(m_Device.m_Device, m_VertexBuffer.m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_VertexBuffer.m_VertexBufferMemory, nullptr);
		for (auto& tex : m_Textures)
		{
			vkDestroySampler(m_Device.m_Device, tex.m_TextureVars.m_Sampler, nullptr);
			vkDestroyImageView(m_Device.m_Device, tex.m_TextureVars.m_TextureView, nullptr);
			vkDestroyImage(m_Device.m_Device, tex.m_TextureVars.m_texture, nullptr);
			vkFreeMemory(m_Device.m_Device, tex.m_TextureVars.m_Memory, nullptr);
		}
		
		
		printf("model Destroyed\n");
	}

	void kModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer vBuffers[] = { m_VertexBuffer.m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void kModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, m_Indices.size(), 1, 0, 0, 0);
	}

	void kModel::loadModel(std::string filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_FlipUVs |
			aiProcess_GenSmoothNormals);

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
		auto numMeshes = node->mNumMeshes;
		auto numMaterials = scene->mNumMaterials;
		printf("number of meshes: %d\n", numMeshes);
		printf("number of materials: %d\n", numMaterials);
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			processNode(mesh, scene);
		}
	}

	void kModel::processNode(aiMesh* mesh, const aiScene *scene)
	{
		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vert;

			vert.pos.x = mesh->mVertices[i].x;
			vert.pos.y = mesh->mVertices[i].y;
			vert.pos.z = mesh->mVertices[i].z;

			vert.normal.x = mesh->mNormals[i].x;
			vert.normal.y = mesh->mNormals[i].y;
			vert.normal.z = mesh->mNormals[i].z;

			vert.texcoords.x = mesh->mTextureCoords[0][i].x;
			vert.texcoords.y = mesh->mTextureCoords[0][i].y;

			vert.color = { 1.0f, 1.0f, 0.0f };

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
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* material = scene->mMaterials[i];

			aiString Matname = material->GetName();
			printf("name of material: %s\n", Matname.C_Str());

			auto numProps = material->mNumProperties;
			printf("number of properties: %d\n", numProps);
			for (auto j = 0; j < material->mNumProperties; j++)
			{
				aiMaterialProperty* prop = material->mProperties[j];
				aiString key = prop->mKey;
				auto sema = prop->mSemantic;
				printf("key %s\n", key);
				printf("semantic %d\n", sema);
			}
		}
		if (mesh->mMaterialIndex >= 0)
		{
			aiString baseColor, metallicRoughness, normalmap, ao, emissive;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiTextureType type = aiTextureType_NORMALS;
			aiTextureType type2 = aiTextureType_AMBIENT_OCCLUSION;
			auto i = material->GetTextureCount(type);
			auto j = material->GetTextureCount(type2);
			printf("count of normal textures: %f", i);
			printf("count of ao textures: %f", j);
			material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &baseColor);
			material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughness);
			material->GetTexture(aiTextureType_NORMALS, 0, &normalmap);
			material->GetTexture(aiTextureType_LIGHTMAP, 0, &ao);
			material->GetTexture(aiTextureType_EMISSIVE, 0, &emissive);
			//material->GetTexture(AI_MATKEY_COLOR_AMBIENT, &ao);
			float normalScale = 1.0f;
			//material->Get(AI_MATKEY_GLTF_NORMALS_SCALE)
			//material->GetTexture(AI_MATKEY_TEXTURE_NORMALS, &normal)
			printf("helo\n");
			printf("BaseColor: %s \n", baseColor.C_Str());
			printf("metallicRoughness: %s \n", metallicRoughness.C_Str());
			printf("normal: %s \n", normalmap.C_Str());
			printf("ambient occlusion: %s \n", ao.C_Str());
			kTexture base{ m_Device, m_SwapChain, baseColor.C_Str(), VK_FORMAT_R8G8B8A8_SRGB};
			kTexture normal{ m_Device, m_SwapChain, normalmap.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };
			kTexture mr{ m_Device, m_SwapChain, metallicRoughness.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };
			kTexture amoc{ m_Device, m_SwapChain, ao.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };
			kTexture em{ m_Device, m_SwapChain, emissive.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };

			m_Textures.push_back(base);
			m_Textures.push_back(normal);
			m_Textures.push_back(mr);
			m_Textures.push_back(amoc);
			m_Textures.push_back(em);
		}
	}

	void kModel::createVertexBuffer(VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		VK_CHECK(vkMapMemory(m_Device.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
		memcpy(data, m_Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.m_Device, stagingBufferMemory);

		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VertexBuffer.m_VertexBuffer, m_VertexBuffer.m_VertexBufferMemory);

		m_Device.copyBuffers(stagingBuffer, m_VertexBuffer.m_VertexBuffer, bufferSize, commandPool);

		vkDestroyBuffer(m_Device.m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, stagingBufferMemory, nullptr);
	}

	void kModel::createVertexBuffer(std::vector<Vertex>& vertices, VkCommandPool commandPool)
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

	void kModel::createIndexBuffer(VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		VK_CHECK(vkMapMemory(m_Device.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
		memcpy(data, m_Indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.m_Device, stagingBufferMemory);

		m_Device.createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_IndexBuffer.m_IndexBuffer, m_IndexBuffer.m_IndexBufferMemory);

		m_Device.copyBuffers(stagingBuffer, m_IndexBuffer.m_IndexBuffer, bufferSize, commandPool);

		vkDestroyBuffer(m_Device.m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device.m_Device, stagingBufferMemory, nullptr);
	}
	void kModel::createIndexBuffer(std::vector<uint32_t>& indices, VkCommandPool commandPool)
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