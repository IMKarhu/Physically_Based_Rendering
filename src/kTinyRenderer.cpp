#include "kTinyRenderer.hpp"
#include "kBuffer.hpp"


#include "utils/vkUtils.hpp"
#include "types.hpp"
#include <chrono>

namespace engine
{
	TinyRenderer::TinyRenderer()
	{
	}
	TinyRenderer::~TinyRenderer()
	{
		cleanUpSwapChain();
		for (size_t i = 0; i < m_MaxFramesInFlight; i++)
		{
			vkDestroyBuffer(m_Device.m_Device, m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device.m_Device, m_UniformBuffersMemory[i], nullptr);
		}
		//vkDestroyBuffer(m_Device.m_Device, m_UniformBuffers[i], nullptr);
		vkFreeMemory(m_Device.m_Device, m_DepthImageMemory, nullptr);
		vkDestroyDescriptorSetLayout(m_Device.m_Device, m_DescriptorLayout, nullptr);
		for (size_t i = 0; i < 2; i++)
		{
			vkDestroySemaphore(m_Device.m_Device, m_Semaphores.availableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.m_Device, m_Semaphores.finishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device.m_Device, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_Device.m_Device, m_CommandPool, nullptr);

		vkDestroySurfaceKHR(m_Window->getInstance(), m_Surface, nullptr);
	}
	void TinyRenderer::buildVulkan(uint32_t maxFramesInFlight, std::vector<karhu::vkglTFModel *>& models)
	{
		setupDebugMessenger();
		m_MaxFramesInFlight = maxFramesInFlight;
		//createSurface();
		//m_Device.m_Instance = m_Window->getInstance();
		//m_Device.m_Surface = m_Surface;
		m_Device.init();
		
		m_SwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
		m_SwapChain.createImageViews();
		m_Pipeline.createRenderPass(m_SwapChain.m_SwapChainImageFormat);

		m_Descriptors.addBind(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		m_Descriptors.addBind(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

		m_DescriptorLayout = m_Descriptors.createDescriptorSetLayout(m_DescriptorLayout);

		
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createFrameBuffers();

		/*create texture
		  create imageview
		  create texture sampler.
		 */
		 models[0]->m_Texture.createTexture(m_CommandPool);
		 models[0]->m_Texture.createTextureImageView();
		 models[0]->m_Texture.createSampler();

		 /* Load gltf models.*/
		for (const auto& model : models)
		{
			std::vector<karhu::Vertex> m_Vertices;
			    std::vector<uint32_t> m_Indices;
				model->loadgltfFile("../models/Cube.gltf", m_Indices, m_Vertices);

			    karhu::Buffer buffer = { m_Device };
			    buffer.createVkBuffer<karhu::Vertex>(m_Vertices, model->m_VertexBuffer.m_Buffer, model->m_VertexBuffer.m_BufferMemory,
			        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_CommandPool);

			    buffer.createVkBuffer<uint32_t>(m_Indices, model->m_IndexBuffer.m_Buffer, model->m_IndexBuffer.m_BufferMemory,
			        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_CommandPool);
		}
		createUniformBuffers(maxFramesInFlight);

		m_DescriptorSets = m_Descriptors.createDescriptorSets(maxFramesInFlight, m_DescriptorLayout, m_UniformBuffers, models[0]->m_Texture);

		createCommandBuffers(maxFramesInFlight);
		createSyncObjects(maxFramesInFlight);
	}
	void TinyRenderer::draw(uint32_t currentFrame, karhu::vkglTFModel *model)
	{
		vkWaitForFences(m_Device.m_Device, 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult res = vkAcquireNextImageKHR(m_Device.m_Device, m_SwapChain.m_SwapChain, UINT64_MAX, m_Semaphores.availableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			reCreateSwapChain();
			return;
		}
		else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to get swapchain image!\n");
		}

		vkResetFences(m_Device.m_Device, 1, &m_InFlightFences[currentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[currentFrame], 0);
		recordCommandBuffer(m_CommandBuffers[currentFrame], imageIndex, model);

		updateUBOs(imageIndex);

		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_Semaphores.availableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitinfo.waitSemaphoreCount = 1;
		submitinfo.pWaitSemaphores = waitSemaphores;
		submitinfo.pWaitDstStageMask = waitStages;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &m_CommandBuffers[currentFrame];
		VkSemaphore signalSemaphores[] = { m_Semaphores.finishedSemaphores[currentFrame] };
		submitinfo.signalSemaphoreCount = 1;
		submitinfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK(vkQueueSubmit(m_Device.m_GraphicsQueue, 1, &submitinfo, m_InFlightFences[currentFrame]));

		VkPresentInfoKHR presentinfo{};
		presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { m_SwapChain.m_SwapChain };
		presentinfo.swapchainCount = 1;
		presentinfo.pSwapchains = swapChains;
		presentinfo.pImageIndices = &imageIndex;
		presentinfo.pResults = nullptr;

		res = vkQueuePresentKHR(m_Device.m_PresentQueue, &presentinfo);

		if (res == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || m_Window->getResize())
		{
			m_Window->setResize(false);
			reCreateSwapChain();
		}
		else if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swapchain image1\n");
		}
		currentFrame = (currentFrame + 1) % m_MaxFramesInFlight;
	}
	void TinyRenderer::update(uint32_t currentFrame, karhu::vkglTFModel* model)
	{
		while (!m_Window->shouldClose())
		{
			m_Window->pollEvents();
			draw(currentFrame, model);
		}
		vkDeviceWaitIdle(m_Device.m_Device);
	}
	void TinyRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index, karhu::vkglTFModel *model)
	{
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = 0;
		info.pInheritanceInfo = nullptr;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &info));

		VkRenderPassBeginInfo renderpassInfo{};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = m_Pipeline.getRenderPass();
		renderpassInfo.framebuffer = m_FrameBuffers[index];
		renderpassInfo.renderArea.offset = { 0,0 };
		renderpassInfo.renderArea.extent = m_SwapChain.m_SwapChainExtent;

		std::array<VkClearValue, 2> clearColors{};
		clearColors[0].color = { {0.0f ,0.0f ,0.0f ,1.0f} };
		clearColors[1].depthStencil = { 1.0f, 0 };
		renderpassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderpassInfo.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.getPipeline());

		VkViewport viewPort{};
		viewPort.x = 0.0f;
		viewPort.y = 0.0f;
		viewPort.width = static_cast<float>(m_SwapChain.m_SwapChainExtent.width);
		viewPort.height = static_cast<float>(m_SwapChain.m_SwapChainExtent.height);
		viewPort.minDepth = 0.0f;
		viewPort.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChain.m_SwapChainExtent;

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vBuffers[] = { model->m_VertexBuffer.m_Buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, model->m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.getLayout(), 0, 1, &m_DescriptorSets[index], 0, nullptr);

		//VertexCount = 3, we techincally have 3 vertices to draw, instanceCount = 1, used for instance rendering, we use one because we dont have any instances
		//firstVertex = 0 offset into the vertex buffer, defines lowest value of gl_VertexIndex
		//firstInstance = 0 offset of instance, defines lowest value of gl_VertexIndex.
		//vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
		model->draw(commandBuffer, m_Pipeline.getLayout(), m_DescriptorSets, index);
		//vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VK_CHECK(vkEndCommandBuffer(commandBuffer));
	}
	void TinyRenderer::createGraphicsPipeline()
	{
		karhu::GraphicsPipelineCreateStruct pipelineStruct;
		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout;

		m_Pipeline.createGraphicsPipeline(pipelineStruct);
	}
	void TinyRenderer::createSurface()
	{
	}
	void TinyRenderer::createCommandPool()
	{
		karhu::QueueFamilyIndices indices = m_Device.findQueueFamilies(m_Device.m_PhysicalDevice);

		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createinfo.queueFamilyIndex = indices.graphicsFamily.value();

		VK_CHECK(vkCreateCommandPool(m_Device.m_Device, &createinfo, nullptr, &m_CommandPool));
	}
	void TinyRenderer::createCommandBuffers(uint32_t maxFramesInFlight)
	{
		m_CommandBuffers.resize(maxFramesInFlight);

		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = m_CommandPool;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		VK_CHECK(vkAllocateCommandBuffers(m_Device.m_Device, &allocinfo, m_CommandBuffers.data()));

	}
	void TinyRenderer::createDepthResources()
	{
		VkFormat format = findDepthFormat();
		karhu::Texture texture{ m_Device };
		texture.createImage(m_SwapChain.m_SwapChainExtent.width, m_SwapChain.m_SwapChainExtent.height, format, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
		m_DepthImageView = texture.createImageview(m_DepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

	}
	void TinyRenderer::createFrameBuffers()
	{
		m_FrameBuffers.resize(m_SwapChain.m_SwapChainImageViews.size());

		for (size_t i = 0; i < m_SwapChain.m_SwapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = {
				m_SwapChain.m_SwapChainImageViews[i],
				m_DepthImageView
			};


			VkFramebufferCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createinfo.renderPass = m_Pipeline.getRenderPass();
			createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createinfo.pAttachments = attachments.data();
			createinfo.width = m_SwapChain.m_SwapChainExtent.width;
			createinfo.height = m_SwapChain.m_SwapChainExtent.height;
			createinfo.layers = 1;

			VK_CHECK(vkCreateFramebuffer(m_Device.m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
		}
	} //engine

	VkFormat TinyRenderer::findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkFormat engine::TinyRenderer::findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : formats)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_Device.m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	void TinyRenderer::createUniformBuffers(uint32_t maxFramesInFlight)
	{
		karhu::Buffer buffer = { m_Device };
		VkDeviceSize bufferSize = sizeof(karhu::UniformBufferObject);

		m_UniformBuffers.resize(maxFramesInFlight);
		m_UniformBuffersMemory.resize(maxFramesInFlight);
		m_UniformBuffersMapped.resize(maxFramesInFlight);

		for (size_t i = 0; i < maxFramesInFlight; i++)
		{
			buffer.createBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_UniformBuffers[i], m_UniformBuffersMemory[i]);
			vkMapMemory(m_Device.m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
		}
	}

	void TinyRenderer::createSyncObjects(uint32_t maxFramesinFlight)
	{
		m_Semaphores.availableSemaphores.resize(maxFramesinFlight);
		m_Semaphores.finishedSemaphores.resize(maxFramesinFlight);
		m_InFlightFences.resize(maxFramesinFlight);

		VkSemaphoreCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceinfo{};
		fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < maxFramesinFlight; i++)
		{
			if (vkCreateSemaphore(m_Device.m_Device, &createinfo, nullptr, &m_Semaphores.availableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device.m_Device, &createinfo, nullptr, &m_Semaphores.finishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device.m_Device, &fenceinfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Semaphores!");
			}
		}
	}
	void TinyRenderer::cleanUpSwapChain()
	{
		vkDestroyImageView(m_Device.m_Device, m_DepthImageView, nullptr);
		vkDestroyImage(m_Device.m_Device, m_DepthImage, nullptr);
		vkFreeMemory(m_Device.m_Device, m_DepthImageMemory, nullptr);
		for (auto framebuffer : m_FrameBuffers)
		{
			vkDestroyFramebuffer(m_Device.m_Device, framebuffer, nullptr);
		}
		for (auto imageView : m_SwapChain.m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device.m_Device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_Device.m_Device, m_SwapChain.m_SwapChain, nullptr);
	}
	void TinyRenderer::reCreateSwapChain()
	{
		VK_CHECK(vkDeviceWaitIdle(m_Device.m_Device));

		cleanUpSwapChain();

		m_SwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
		m_SwapChain.createImageViews();
		createDepthResources();
		createFrameBuffers();
	}
	void TinyRenderer::updateUBOs(uint32_t index)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		karhu::UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.m_SwapChainExtent.width / (float)m_SwapChain.m_SwapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		memcpy(m_UniformBuffersMapped[index], &ubo, sizeof(ubo));
	}
	void TinyRenderer::setupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createinfo{};
		karhu::vkUtils::populateDebugMessengerCreateInfo(createinfo);

		if (createDebugUtilsMessengerEXT(m_Window->getInstance(), &createinfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create debug messenger!\n");
		}
	}

	VkResult TinyRenderer::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void TinyRenderer::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}
