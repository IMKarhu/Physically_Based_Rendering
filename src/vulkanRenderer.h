#pragma once

#include "window.h"
#include "device.h"
#include "swapChain.h"
#include "graphicsPipeLine.h"

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
public:
	VulkanRenderer();
	~VulkanRenderer();

	void Run();
private:
	std::unique_ptr<Window> m_Window = std::make_unique<Window>("Vulkan renderer", 800, 600);
	Device* m_Device;
	SwapChain* m_SwapChain;
	GraphicsPipeLine* m_PipeLine;
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkCommandPool m_CommandPool;
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