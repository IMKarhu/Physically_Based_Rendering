#include "kSwapChain.hpp"
#include "utils/macros.hpp"

#include <limits>
#include <algorithm>
#include <array>

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
		vkDestroyCommandPool(m_Device.m_Device, m_CommandPool, nullptr);
		vkDestroyRenderPass(m_Device.m_Device, m_RenderPass[NORMAL], nullptr);
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

	VkImageView Vulkan_SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t layerCount)
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
		viewInfo.subresourceRange.layerCount = layerCount;

		VkImageView imageView;
		VK_CHECK(vkCreateImageView(m_Device.m_Device, &viewInfo, nullptr, &imageView));

		return imageView;
	}

	void Vulkan_SwapChain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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

	void Vulkan_SwapChain::createRenderPass(VkFormat depthFormat, renderPassType type)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (type == CUBE)
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		VkAttachmentDescription depthAttachment{};
		if (type != CUBE)
		{
			depthAttachment.format = depthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		if (type != CUBE)
		{
			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}


		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		if (type != CUBE)
		{
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		

		
		
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		if (type != CUBE)
		{
			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
		}
		else {
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
		}
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (type != CUBE)
		{
			VK_CHECK(vkCreateRenderPass(m_Device.m_Device, &renderPassInfo, nullptr, &m_RenderPass[NORMAL]));
		}
		else {
			VK_CHECK(vkCreateRenderPass(m_Device.m_Device, &renderPassInfo, nullptr, &m_RenderPass[CUBE]));
		}
		
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