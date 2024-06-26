#include "kSwapChain.hpp"
#include "utils/macros.hpp"

#include <limits>
#include <algorithm>

namespace karhu
{
	Vulkan_SwapChain::Vulkan_SwapChain(VkDevice device, GLFWwindow* window)
	{
		m_Device = device;
		m_Window = window;
		printf("constructor called!\n");
	}

	Vulkan_SwapChain::~Vulkan_SwapChain()
	{
		printf("swapchain dsetroyed\n");
		/*for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}*/
	}

	VkSurfaceFormatKHR Vulkan_SwapChain::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}
		return formats[0];
	}

	VkPresentModeKHR Vulkan_SwapChain::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& presentMode : presentModes)
		{
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Vulkan_SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_Window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void Vulkan_SwapChain::createSwapChain(SwapChainSupportDetails swapChainSupport, VkSurfaceKHR surface, VkSwapchainCreateInfoKHR createInfo)
	{
		SwapChainSupportDetails swapchainSupport = swapChainSupport;

		VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
		{
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createinfo.surface = surface;
		createinfo.minImageCount = imageCount;
		createinfo.imageFormat = surfaceFormat.format;
		createinfo.imageColorSpace = surfaceFormat.colorSpace;
		createinfo.imageExtent = extent;
		createinfo.imageArrayLayers = 1;
		createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createinfo.imageSharingMode = createInfo.imageSharingMode;
		createinfo.queueFamilyIndexCount = createInfo.queueFamilyIndexCount;
		createinfo.pQueueFamilyIndices = createInfo.pQueueFamilyIndices;
		createinfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createinfo.presentMode = presentMode;
		createinfo.clipped = VK_TRUE;
		createinfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(m_Device, &createinfo, nullptr, &m_SwapChain));

		VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr));
		m_SwapChainImages.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data()));

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void Vulkan_SwapChain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createinfo.image = m_SwapChainImages[i];
			createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createinfo.format = m_SwapChainImageFormat;
			createinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createinfo.subresourceRange.baseMipLevel = 0;
			createinfo.subresourceRange.levelCount = 1;
			createinfo.subresourceRange.baseArrayLayer = 0;
			createinfo.subresourceRange.layerCount = 1;

			VK_CHECK(vkCreateImageView(m_Device, &createinfo, nullptr, &m_SwapChainImageViews[i]));
		}
	}

}