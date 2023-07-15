#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>



const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

class Instance
{
public:
	Instance(bool enablevalidationLayers);
	Instance() : Instance(true){} /* TODO: Fix this to take parameter rather than just a simple true boolean. */

	Instance(const Instance&) = delete; /* Disallows copying. */
	Instance(Instance&&) noexcept;

	~Instance();

	Instance& operator=(const Instance&) = delete;
	Instance& operator=(Instance&&) = default;

	bool checkValidationLayers();
	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
private:
	VkInstance m_Instance{ VK_NULL_HANDLE };

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};