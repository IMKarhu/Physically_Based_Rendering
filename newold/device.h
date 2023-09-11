#pragma once
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>
#include <set>

namespace kge
{
	class Device
	{
	public:
		Device(const VkInstance& instance, GLFWwindow* window, const bool& enableValidationLayers);
		~Device();

		void createSurface(const VkInstance& instance, GLFWwindow* window);
		void pickPhysicalDevice(const VkInstance& instance);
		void createLogicalDevice(const bool& enableValidationLayers);
		bool isDeviceSuitable(VkPhysicalDevice pdevice);
		bool checkDeviceExtensionSupport(VkPhysicalDevice pdevice);

		[[nodiscard]] const VkDevice getDevice() const { return m_Device; }
		[[nodiscard]] const VkPhysicalDevice getPDevice() const { return m_PhysicalDevice; }
		[[nodiscard]] const VkSurfaceKHR getSurface() const { return m_Surface; }
		[[nodiscard]] const VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] const VkQueue getPresentQueue() const { return m_PresentQueue; }

	private:
		/* Class members. */
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface;
		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};
	};
}