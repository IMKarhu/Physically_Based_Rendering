#pragma once

#include "window.h"
#include "device.h"
#include "swapChain.h"

#include <vector>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
//const std::vector<const char*> deviceExtensions = {
//	VK_KHR_SWAPCHAIN_EXTENSION_NAME
//};

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
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;

	/* Private member Functions. */
	void createInstance();
	void setupDebugMessenger();

	bool checkValidationSupport() const;
	std::vector<const char*> getRequiredExtensions() const;
	//VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
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