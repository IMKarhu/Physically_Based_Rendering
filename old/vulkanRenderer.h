#pragma once

#include "window.h"
#include "device.h"
#include "swapChain.h"
#include "graphicsPipeLine.h"
#include "buffer.h"
#include "texture.h"
#include "mesh.h"

#include <vector>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class VulkanRenderer
{
	//friend class Buffer;
public:
	VulkanRenderer();
	~VulkanRenderer();

	void Run();
private:
	std::unique_ptr<Window> m_Window = std::make_unique<Window>("Vulkan renderer", 800, 600);
	Device* m_Device;
	SwapChain* m_SwapChain;
	GraphicsPipeLine* m_PipeLine;
	Buffer* m_Buffer;
	Texture* m_Texture;
	std::vector<Mesh*> m_Meshes;
	//Mesh* m_Mesh;

	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkCommandPool m_CommandPool;

	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;

	const int m_MaxFramesInFlight = 2;
	uint32_t m_CurrentFrame = 0;


	/* Private member Functions. */
	void createInstance();
	void setupDebugMessenger();
	void createCommandPool();
	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandbuffer, uint32_t imageIndex);
	void createSyncObjects();
	void drawFrame();
	void recreateSwapChain();
	void cleanUpSwapChain();
	Mesh* processMesh(aiMesh* mesh);
	void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene);
	std::vector<Mesh*> loadMesh(const std::string& path);

	void createVertexBuffers();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); /* Refactor this into own abstarct class. */
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	bool checkValidationSupport() const;
	std::vector<const char*> getRequiredExtensions() const;
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};