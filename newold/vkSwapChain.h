#pragma once
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <vector>

namespace kge
{
	class vkSwapChain
	{
	public:
		vkSwapChain();
		~vkSwapChain();
		void createSwapChain(const VkDevice& device, const VkPhysicalDevice& pdevice, const VkSurfaceKHR& surface, GLFWwindow* window);
		void createSwapChainImageViews(const VkDevice& device);

		[[nodiscard]] const VkSwapchainKHR getSwapChain() const { return m_SwapChain; }
		[[nodiscard]] const std::vector<VkImageView> getSCImageViews() const { return m_SwapChainImageViews; }
		[[nodiscard]] const VkFormat getSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		[[nodiscard]] const VkExtent2D getSwapChainExtent() const { return m_SwapChainExtent; }
	private:
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;
		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainExtent{};
	};
}
