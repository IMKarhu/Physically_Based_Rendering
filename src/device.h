#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device
{
public:
	Device(VkInstance instance,GLFWwindow* window, bool validationlayers, const std::vector<const char*> validationLayersvector);

	~Device();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pdevice);
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	bool checkDeviceExtensionSupport();

	bool isDeviceSuitable(VkPhysicalDevice pdevice);

	VkDevice getDevice() const { return m_Device; }
private:
	std::vector<const char*> m_ValidationLayers;
	bool m_Enablevalidationlayers = false;
	VkInstance m_Instance{};
	GLFWwindow* m_Window;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface{};
	VkDevice m_Device{};
	VkQueue m_GraphicsQueue{};
	VkQueue m_PresentQueue{};

	
};