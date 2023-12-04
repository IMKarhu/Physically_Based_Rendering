#include "vkApplication.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <array>




namespace kge
{

	vkApplication::vkApplication()
	{
		std::cout << "Default Constructor called!\n" << std::endl;
	}

	vkApplication::~vkApplication()
	{
		std::cout << "Destructor called!\n" << std::endl;
		vkDestroyPipelineLayout(m_Device->getDevice(), m_PipelineLayout, nullptr);
		delete m_DescriptorLayout;
		delete m_Model;
		delete m_PipeLine;
		delete m_SwapChain;
		delete m_Device;
		m_Instance->cleanUp(enableValidationLayers);
		delete m_Instance;
	}

	void vkApplication::run()
	{
		std::cout << "Run!\n" << std::endl;
		initVulkan();
		while (!m_Instance->shouldClose())
		{
			m_Instance->pollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(m_Device->getDevice());
	}

	void vkApplication::drawFrame()
	{
		
		vkWaitForFences(m_Device->getDevice(), 1, &m_SwapChain->getInFlightFences()[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device->getDevice(), m_SwapChain->getSwapChain(), UINT64_MAX, m_SwapChain->getImgSemaphores()[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		//VkResult result = m_SwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_SwapChain->recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swapchain image!");
		}

		m_Model->updateUniformBuffer(m_CurrentFrame,m_SwapChain->getWidth(),m_SwapChain->getHeight());

		vkResetFences(m_Device->getDevice(), 1, &m_SwapChain->getInFlightFences()[m_CurrentFrame]);
		m_SwapChain->resetCommandBuffer(m_CommandBuffers, m_CurrentFrame);
		recordCommandBuffers(m_CommandBuffers[m_CurrentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_SwapChain->getImgSemaphores()[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

		VkSemaphore signalSemaphores[] = { m_SwapChain->getFinishedSemaphores()[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		
		if (vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_SwapChain->getInFlightFences()[m_CurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain->getSwapChain()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(m_Device->getPresentQueue(), &presentInfo);

		m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
		//m_SwapChain->createDrawFrame(&imageIndex,m_CommandBuffers);
	}

	void vkApplication::loadModels()
	{
		std::vector<kgeModel::Vertex> vertices =
		{
			 {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			 {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			 {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			 {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		std::vector<uint16_t> indices =
		{
			0,1,2,2,3,0
		};

		m_Model = new kgeModel(m_Device, vertices, indices, m_MaxFramesInFlight);
		m_DescriptorLayout = new vkDescriptorSetLayout(m_Device, m_Model);
		ubobinding = m_DescriptorLayout->createBinding(ubobinding, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		m_DescriptorSetLayout = m_DescriptorLayout->createDescriptorSetLayout(m_DescriptorSetLayout, ubobinding, 1);
		m_DescriptorLayout->createDescriptorPool(m_MaxFramesInFlight);
		m_DescriptorLayout->createDescriptorSets(m_MaxFramesInFlight);
	}
   
	void vkApplication::createPipeLineLayout()
	{
		//m_DescriptorSetLayout = m_DescriptorLayout->createDescriptorSetLayout(m_DescriptorSetLayout, ubobinding, 1);
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		if (vkCreatePipelineLayout(m_Device->getDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void vkApplication::createPipeLine()
	{
		vkPipelineConfigInfo pipeLineConfig{};
		m_PipeLine->createPipeLineConfigInfo(pipeLineConfig, m_SwapChain->getWidth(), m_SwapChain->getHeight());
		pipeLineConfig.renderPass = m_SwapChain->getRenderPass();
		pipeLineConfig.pipeLineLayout = m_PipelineLayout;
		m_PipeLine = new vkPipeline(m_Device, "../shaders/vert.spv", "../shaders/frag.spv",pipeLineConfig);
	}

	void vkApplication::createCommandBuffers()
	{
		m_CommandBuffers.resize(m_SwapChain->getFramesInFlight());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Device->getPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
		
	}

	void vkApplication::recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageindex)
	{
		
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional
			

			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_SwapChain->getRenderPass();
			renderPassInfo.framebuffer = m_SwapChain->getFramebuffer()[imageindex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_SwapChain->getSwapchainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f,0.0f,0.0f,1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			m_PipeLine->bind(commandBuffer);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)m_SwapChain->getWidth();
			viewport.height = (float)m_SwapChain->getHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_SwapChain->getSwapchainExtent();

			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			m_Model->bind(commandBuffer, m_PipelineLayout, m_DescriptorLayout->getDescriptorSets(), m_CurrentFrame);

			m_Model->draw(commandBuffer);

			vkCmdEndRenderPass(commandBuffer);

			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!\n");
			}
	}

	/*void vkApplication::updateUniformBuffer(uint32_t currentFrame)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		kgeModel::UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain->getWidth() / (float)m_SwapChain->getHeight(), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
	}*/

	void vkApplication::initVulkan()
	{
		std::cout << "initVulkan!\n" << std::endl;
		//m_Model = new kgeModel(m_Device, m_Model->loadModels());
		loadModels();
		m_SwapChain->createSwapChain();
		m_SwapChain->createImageViews();
		m_SwapChain->createDepthResources();
		m_SwapChain->createRenderPass();
		//ubobinding = m_DescriptorLayout->createBinding(ubobinding, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
		createPipeLineLayout();
		createPipeLine();
		m_SwapChain->createFrameBuffers();
		//m_Device->createCommandPool();
		createCommandBuffers();
		m_SwapChain->createSyncObjects();
	}
	
}

