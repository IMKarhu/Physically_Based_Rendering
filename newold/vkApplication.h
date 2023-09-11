#pragma once
#include "vkInstance.h"
#include "device.h"
#include "vkSwapChain.h"
#include "vkPipeLine.h"
#include "command.h"
#include "Buffer.h"
#include "uniformBuffers.h"
#include "model/model.h"
#include "descriptor/descriptor.h"
#include "model/mesh.h"

#include <glm/gtx/hash.hpp>
#include "vulkan/vulkan.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <iostream>

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

namespace kge
{
	

	class vkApplication
	{
	public:
		vkApplication();
		vkApplication(const vkApplication&) = delete; /* Disallows copying. */
		vkApplication(vkApplication&&) = delete; /* Disallows move operation. */
		~vkApplication();

		vkApplication& operator=(const vkApplication&) = delete;
		vkApplication& operator=(vkApplication&&) = delete;

		void run();
		void drawFrame();
		void recreateSwapChain();
		void cleanUpSwapChain();

	private:
		/* Pointers to other classes. */
		vkInstance* m_Instance = new vkInstance(enableValidationLayers);
		Device* m_Device = new Device(m_Instance->getInstance(), m_Instance->getWindow(), enableValidationLayers);
		vkSwapChain* m_SwapChain = new vkSwapChain();
		Command* m_Command = new Command();
		UniformBuffers* m_UniformBuffers = new UniformBuffers(m_Command);
		//Buffer* m_Buffer = new Buffer(m_Command);
		Model* m_Model = new Model(m_Command, m_SwapChain, m_UniformBuffers);
		VulkanPipeLine* m_PipeLine = new VulkanPipeLine(m_SwapChain, m_Device, m_Model);
		Descriptor* m_Descriptor = new Descriptor(m_Device);
		/* Class members. */
		uint32_t m_CurrentFrame = 0;
		const int m_MaxFramesInFlight = 2;
		std::vector<Mesh*> m_Meshes;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		/* Private functions. */
		void createSyncObjects();
		Mesh* processMesh(aiMesh* mesh);
		void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene);
		std::vector<Mesh*> loadMesh(const std::string& path);
		void initVulkan();
	};
}
