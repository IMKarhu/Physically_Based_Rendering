#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kGraphicsPipeline.hpp"
#include "kDescriptors.hpp"

#include <memory>
#include <fstream>
#include <chrono>


namespace karhu
{
	//class kModel;
	class kEntity;
	class kCamera;

	class kRenderer
	{
	public:
		kRenderer();
		~kRenderer();

		kRenderer(const kRenderer&) = delete;
		void operator=(const kRenderer&) = delete;

		void createFrameBuffers();
		void recordCommandBuffer(kEntity& entity, uint32_t currentFrameIndex, uint32_t index);
		void beginRecordCommandBuffer(uint32_t currentFrameIndex, uint32_t index);
		void endRecordCommandBuffer(uint32_t currentFrameIndex);
		void createSyncObjects();
		void createUniformBuffers(std::vector<kEntity>& entities);
		void updateUBOs(std::vector<kEntity>& entities, kCamera& camera);
		void beginFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex);
		void endFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex);
		Vulkan_Device& getDevice() { return m_VkDevice; }
		Vulkan_SwapChain& getSwapChain() { return m_VkSwapChain; }
		kDescriptors& getDescriptor() { return m_DescriptorBuilder; }
		VkCommandPool getCommandPool() const { return m_VkSwapChain.m_CommandPool; }
		bool getWindowShouldclose() { return m_Window->shouldClose(); }
		void windowPollEvents() { return m_Window->pollEvents(); }
		GLFWwindow* getWindow() { return m_Window->getWindow(); }
		const int getWindowWidth() const { return m_Window->getWidth(); }
		const int getWindowHeight() const { return m_Window->getheight(); }
	private:
		void createGraphicsPipeline();
		void createDepthResources(); //refactor somewhere else image class?
		VkFormat findDepthFormat(); // refactor somewhere elseimage class?
		bool hasStencilComponent(VkFormat format); //refactor somewhere else iamge class?
		void createImage(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory); //reafctor somewhere else image class?
		void cleanUpSwapChain();
		void reCreateSwapChain();
	private:
		std::unique_ptr<kWindow> m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
		Vulkan_Device m_VkDevice{ m_Window->getInstance(), m_Window->getSurface() };
		Vulkan_SwapChain m_VkSwapChain{ m_VkDevice };
		kGraphicsPipeline m_GraphicsPipeline{ m_VkDevice };
		kDescriptors m_DescriptorBuilder{ m_VkDevice };
		//std::shared_ptr<kDescriptors> m_Descriptor; /*= std::make_shared<kDescriptors>();*/

		VkDescriptorSetLayout m_DescriptorLayout;
		std::vector<VkFramebuffer> m_FrameBuffers;
		struct m_Semaphores
		{
			std::vector<VkSemaphore> availableSemaphores;
			std::vector<VkSemaphore> finishedSemaphores;
		}m_Semaphores;
		std::vector<VkFence> m_InFlightFences;
		float m_DeltaTime = 0.0f;
		uint32_t m_CurrentFrame = 0;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;

		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;
	};
}