#pragma once
#include <vulkan/vulkan.hpp>
#include <set>

class Instance;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device
{
public:

	Device(const VkSurfaceKHR &surface,const VkInstance &instance, bool enablevalidationlayers, std::vector<const char*> validationlayers);
	Device(const Device&) = delete;
	Device(Device&&) noexcept;

	~Device();

	Device& operator=(const Device&) = delete;
	Device& operator=(Device&&) = default;

	void pickPhysicalDevice(const VkSurfaceKHR& surface, const VkInstance& instance);
	bool isDeviceSuitable(const VkSurfaceKHR& surface);
	bool checkDeviceExtensionSupport();
private:
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue = nullptr;
	VkQueue m_PresentQueue = nullptr;

	Instance* m_Instance;
};