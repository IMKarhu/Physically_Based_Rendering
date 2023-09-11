#include "vkSwapChain.h"
#include "vkUtils/swapChainUtils.h"
#include "vkUtils/queuefamilies.h"
#include "vkInstance.h"
#include "vkDevice.h"
#include <array>

namespace kge
{
	vkSwapChain::vkSwapChain(vkInstance* instance, vkDevice* device)
		:m_Instance(instance), m_Device(device)
	{
		
	}

	vkSwapChain::~vkSwapChain()
	{
		std::cout << "SwapChain destructor called!\n" << std::endl;
		cleanUpswapChain();
		
		for (int i = 0; i < m_MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(m_Device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device->getDevice(), m_InFlightFences[i], nullptr);
		}
		
		vkDestroyRenderPass(m_Device->getDevice(), m_RenderPass, nullptr);
	}

	void vkSwapChain::createSwapChain()
	{
		vkUtils::SwapChainSupportDetails swapChainSupport = vkUtils::querySwapChainSupport(m_Device->getPDevice(),m_Instance->getSurface());

		VkSurfaceFormatKHR surfaceFormat = vkUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = vkUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = vkUtils::chooseSwapExtent(swapChainSupport.capabilities,m_Instance->getWindow());

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Instance->getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		vkUtils::QueueFamilyIndices indices = vkUtils::findQueueFamilies(m_Device->getPDevice(), m_Instance->getSurface());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_Device->getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(m_Device->getDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device->getDevice(), m_SwapChain, &imageCount, m_SwapchainImages.data());

		m_swapChainImageFormat = surfaceFormat.format;
		m_SwapchainExtent = extent;
	}

	void vkSwapChain::createImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapchainImages.size());
		for (size_t i = 0; i < m_SwapchainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Device->getDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
	void vkSwapChain::createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_Device->getDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void vkSwapChain::createFrameBuffers()
	{
		m_SwapChainFrameBuffers.resize(m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				m_SwapChainImageViews[i],
				m_DepthImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapchainExtent.width;
			framebufferInfo.height = m_SwapchainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Device->getDevice(), &framebufferInfo, nullptr, &m_SwapChainFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void vkSwapChain::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();
		VkExtent2D swapChainExtent = getSwapchainExtent();

		m_DepthImages.resize(m_SwapchainImages.size());
		m_DepthImageMemorys.resize(m_SwapchainImages.size());
		m_DepthImageViews.resize(m_SwapchainImages.size());

		for (int i = 0; i < m_DepthImages.size(); i++) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = swapChainExtent.width;
			imageInfo.extent.height = swapChainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			m_Device->createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_DepthImages[i],
				m_DepthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_DepthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Device->getDevice(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void vkSwapChain::createSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(m_MaxFramesInFlight);
		m_RenderFinishedSemaphores.resize(m_MaxFramesInFlight);
		m_InFlightFences.resize(m_MaxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < m_MaxFramesInFlight; i++)
		{
			if (vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkResult vkSwapChain::acquireNextImage(uint32_t* imageindex, uint32_t currentFrame)
	{
		vkWaitForFences(m_Device->getDevice(), 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		
		VkResult result = vkAcquireNextImageKHR(m_Device->getDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageindex);
		return result;
	}

	void vkSwapChain::resetCommandBuffer(std::vector<VkCommandBuffer> commandBuffers, uint32_t currentFrame)
	{
		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	}

	VkFormat vkSwapChain::findDepthFormat()
	{
		return  m_Device->findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	void vkSwapChain::recreateSwapChain()
	{
		int width = 0;
		int height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_Instance->getWindow(), &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_Device->getDevice());

		cleanUpswapChain();

		createSwapChain();
		createImageViews();
		createDepthResources();
		createFrameBuffers();
	}

	void vkSwapChain::cleanUpswapChain()
	{
		for (auto depthimageView : m_DepthImageViews)
		{
			vkDestroyImageView(m_Device->getDevice(), depthimageView, nullptr);
		}
		for (auto depthimage : m_DepthImages)
		{
			vkDestroyImage(m_Device->getDevice(), depthimage, nullptr);
		}
		for (auto depthimagememory : m_DepthImageMemorys)
		{
			vkFreeMemory(m_Device->getDevice(), depthimagememory, nullptr);
		}
		for (auto framebuffer : m_SwapChainFrameBuffers) {
			vkDestroyFramebuffer(m_Device->getDevice(), framebuffer, nullptr);
		}
		for (auto imageView : m_SwapChainImageViews) {
			vkDestroyImageView(m_Device->getDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_Device->getDevice(), m_SwapChain, nullptr);
	}

	//void vkSwapChain::createDrawFrame(uint32_t *imageIndex, std::vector<VkCommandBuffer> commandBuffers)
	//{
	//	/*if (m_ImagesInFlight[currentFrame] != VK_NULL_HANDLE)
	//	{*/
	//		//vkWaitForFences(m_Device->getDevice(), 1, &m_ImagesInFlight[currentFrame], VK_TRUE, UINT64_MAX);
	//	//}
	//	//

	//	VkSubmitInfo submitInfo{};
	//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[currentFrame] };
	//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	//	submitInfo.waitSemaphoreCount = 1;
	//	submitInfo.pWaitSemaphores = waitSemaphores;
	//	submitInfo.pWaitDstStageMask = waitStages;

	//	submitInfo.commandBufferCount = 1;
	//	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	//	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[currentFrame] };
	//	submitInfo.signalSemaphoreCount = 1;
	//	submitInfo.pSignalSemaphores = signalSemaphores;

	//	vkResetFences(m_Device->getDevice(), 1, &m_InFlightFences[currentFrame]);
	//	if (vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[currentFrame]) != VK_SUCCESS) {
	//		throw std::runtime_error("failed to submit draw command buffer!");
	//	}

	//	VkPresentInfoKHR presentInfo{};
	//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	//	presentInfo.waitSemaphoreCount = 1;
	//	presentInfo.pWaitSemaphores = signalSemaphores;

	//	VkSwapchainKHR swapChains[] = { m_SwapChain };
	//	presentInfo.swapchainCount = 1;
	//	presentInfo.pSwapchains = swapChains;

	//	presentInfo.pImageIndices = imageIndex;

	//	vkQueuePresentKHR(m_Device->getPresentQueue(), &presentInfo);

	//	currentFrame = (currentFrame + 1) % m_MaxFramesInFlight;
	//}
}