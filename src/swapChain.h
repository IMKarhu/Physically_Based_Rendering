#pragma once
#include "vkUtils/vkUtils.h"
#include "device.h"

class Device;

class SwapChain
{
public:
	SwapChain(Device* device);
	~SwapChain();

	void createSwapChain(GLFWwindow* window);
	void createImageViews();
	

	 [[nodiscard]] const VkFormat getSwapChainImageFormat() { return m_SwapChainImageFormat; }
	 [[nodiscard]] const std::vector<VkImageView> getSwapChainImageViews() { return m_SwapChainImageViews; }
	 [[nodiscard]] const VkExtent2D getSwapChainExtent() { return m_SwapChainExtent; }
	 [[nodiscard]] const VkSwapchainKHR getSwapChain() { return m_SwapChain; }
private:
	VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat{};
	VkExtent2D m_SwapChainExtent{};
	std::vector<VkImageView> m_SwapChainImageViews;
	Device* m_Device;
};