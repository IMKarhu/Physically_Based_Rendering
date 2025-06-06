#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kGraphicsPipeline.hpp"
#include "kDescriptors.hpp"
#include "frame.hpp"


#include <memory>
#include <fstream>
#include <chrono>


namespace karhu
{
	class kEntity;
	class kCamera;
	struct kBuffer;

	class kRenderer
	{
	public:
		kRenderer();
		~kRenderer();

		kRenderer(const kRenderer&) = delete;
		void operator=(const kRenderer&) = delete;

		void createFrameBuffers();
		void recordCommandBuffer(glm::vec3 position, glm::vec3 lightPos, glm::vec4 lightColor, Frame& frameInfo);
		void createSyncObjects();
		void createUniformBuffers(kBuffer& buffer);
		void updateUBOs(std::vector<std::unique_ptr<kBuffer>>& buffers, kCamera& camera);
		void updateObjBuffers(std::vector<kBuffer>& buffers, kEntity& entity);
		VkCommandBuffer beginFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex);
		void endFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex, VkCommandBuffer commandBuffer);
		void renderImguiLayer(VkCommandBuffer commandBuffer, Frame& frameInfo, float dt);
		Vulkan_Device& getDevice() { return m_VkDevice; }
		Vulkan_SwapChain& getSwapChain() { return m_VkSwapChain; }
		VkCommandPool getCommandPool() const { return m_VkSwapChain.m_CommandPool; }
		bool getWindowShouldclose() { return m_Window->shouldClose(); }
		void windowPollEvents() { return m_Window->pollEvents(); }
		GLFWwindow* getWindow() { return m_Window->getWindow(); }
		const int getWindowWidth() const { return m_Window->getWidth(); }
		const int getWindowHeight() const { return m_Window->getheight(); }
	private:
		
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
		void initializeImGui();
	private:
		std::unique_ptr<kWindow> m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
		Vulkan_Device m_VkDevice{ m_Window->getInstance(), m_Window->getSurface() };
		Vulkan_SwapChain m_VkSwapChain{ m_VkDevice };
		kGraphicsPipeline m_GraphicsPipeline{ m_VkDevice };

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
		VkDescriptorPool m_ImguiPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		struct {
			float m_Metalness = 0.0f;
			float m_Roughness = 0.0f;
			glm::vec3 m_LightPosition = glm::vec3(1.0f, 3.0f, 1.0f);
		}vars;
	};
}