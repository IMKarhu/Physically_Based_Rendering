#pragma once
#include "utils/vkUtils.hpp"
#include <vector>

namespace karhu
{
	struct Vulkan_Device;

	struct Vulkan_SwapChain
	{
		Vulkan_SwapChain() = default;
		Vulkan_SwapChain(std::shared_ptr<Vulkan_Device>& device, GLFWwindow* window);
		~Vulkan_SwapChain();

		VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void createSwapChain(VkSurfaceKHR surface);
		void createImageViews();

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainExtent{};

		std::vector<VkImageView> m_SwapChainImageViews;
	private:
		GLFWwindow* m_Window;
		std::shared_ptr<Vulkan_Device> m_VkDevice;
	};
}