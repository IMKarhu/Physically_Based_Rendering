#include "kSwapChain.hpp"
#include "utils/macros.hpp"

#include <limits>
#include <algorithm>

namespace karhu
{
	Vulkan_SwapChain::Vulkan_SwapChain(Vulkan_Device& device)
		:m_Device(device)
	{
		printf("constructor called!\n");
		createCommandPool();
		createCommandBuffers();
	}

	Vulkan_SwapChain::~Vulkan_SwapChain()
	{
		printf("swapchain dsetroyed\n");
		/*for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}*/
		vkDestroyCommandPool(m_Device.m_Device, m_CommandPool, nullptr);
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

	VkExtent2D Vulkan_SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void Vulkan_SwapChain::createSwapChain(VkSurfaceKHR surface, GLFWwindow* window)
	{
		SwapChainSupportDetails swapchainSupport = m_Device.querySwapChainSupport(m_Device.m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities, window);

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

        QueueFamilyIndices indices = m_Device.findQueueFamilies(m_Device.m_PhysicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createinfo.queueFamilyIndexCount = 2;
            createinfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createinfo.queueFamilyIndexCount = 0;
            createinfo.pQueueFamilyIndices = nullptr;
        }

		createinfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createinfo.presentMode = presentMode;
		createinfo.clipped = VK_TRUE;
		createinfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(m_Device.m_Device, &createinfo, nullptr, &m_SwapChain));

		VK_CHECK(vkGetSwapchainImagesKHR(m_Device.m_Device, m_SwapChain, &imageCount, nullptr));
		m_SwapChainImages.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(m_Device.m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data()));

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	VkImageView Vulkan_SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_CHECK(vkCreateImageView(m_Device.m_Device, &viewInfo, nullptr, &imageView));

		return imageView;
	}

	void Vulkan_SwapChain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void Vulkan_SwapChain::createCommandBuffers()
	{
		m_CommandBuffers.resize(m_MaxFramesInFlight);

		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = m_CommandPool;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VK_CHECK(vkAllocateCommandBuffers(m_Device.m_Device, &allocinfo, m_CommandBuffers.data()));
	}

	void Vulkan_SwapChain::createCommandPool()
	{
		QueueFamilyIndices indices = m_Device.findQueueFamilies(m_Device.m_PhysicalDevice);

		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createinfo.queueFamilyIndex = indices.graphicsFamily.value();

		VK_CHECK(vkCreateCommandPool(m_Device.m_Device, &createinfo, nullptr, &m_CommandPool));
	}

	VkCommandBuffer Vulkan_SwapChain::RecordSingleCommand()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VK_CHECK(vkAllocateCommandBuffers(m_Device.m_Device, &allocInfo, &commandBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		return commandBuffer;
	}

	void Vulkan_SwapChain::endSingleCommand(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VK_CHECK(vkQueueSubmit(m_Device.m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK(vkQueueWaitIdle(m_Device.m_GraphicsQueue));
		vkFreeCommandBuffers(m_Device.m_Device, m_CommandPool, 1, &commandBuffer);
	}

}