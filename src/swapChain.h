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
private:
	VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews;
	Device* m_Device;
};