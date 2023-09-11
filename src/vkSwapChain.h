#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace kge
{
	class vkInstance;
	class vkDevice;

	class vkSwapChain
	{
	public:
		vkSwapChain(vkInstance* instance, vkDevice* device);
		vkSwapChain(const vkSwapChain&) = delete; /* Disallows copying. */
		~vkSwapChain();

		vkSwapChain& operator=(const vkSwapChain&) = delete;

		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFrameBuffers();
		void createDepthResources();
		void createSyncObjects();
		VkResult acquireNextImage(uint32_t* imageindex, uint32_t currentFrame);
		void resetCommandBuffer(std::vector<VkCommandBuffer> commandBuffers, uint32_t currentFrame);
		VkFormat findDepthFormat();
		void recreateSwapChain();
		void cleanUpswapChain();
		//void createDrawFrame(uint32_t *imageIndex, std::vector<VkCommandBuffer> commandBuffers);

		[[nodiscard]] const VkRenderPass getRenderPass() const { return m_RenderPass; }
		[[nodiscard]] const uint32_t getWidth() const { return m_SwapchainExtent.width; }
		[[nodiscard]] const uint32_t getHeight() const { return m_SwapchainExtent.height; }
		[[nodiscard]] const int getFramesInFlight() const { return m_MaxFramesInFlight; }
		[[nodiscard]] const std::vector<VkFramebuffer> getFramebuffer() const { return m_SwapChainFrameBuffers; }
		[[nodiscard]] const VkExtent2D getSwapchainExtent() const { return m_SwapchainExtent; }
		//[[nodiscard]] const size_t getCurrentFrame() const { return currentFrame; }
		[[nodiscard]] const VkSwapchainKHR getSwapChain() const { return m_SwapChain; }
		[[nodiscard]] const std::vector<VkSemaphore> getImgSemaphores() const { return m_ImageAvailableSemaphores; }
		[[nodiscard]] const std::vector<VkSemaphore> getFinishedSemaphores() const { return m_RenderFinishedSemaphores; }
		[[nodiscard]] const std::vector<VkFence> getInFlightFences() const { return m_InFlightFences; }
	private:
		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapChainImageViews;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_SwapchainExtent;
		VkRenderPass m_RenderPass;
		std::vector<VkFramebuffer> m_SwapChainFrameBuffers;

		std::vector<VkImage> m_DepthImages;
		std::vector<VkDeviceMemory> m_DepthImageMemorys;
		std::vector<VkImageView> m_DepthImageViews;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		//size_t currentFrame = 0;
		const int m_MaxFramesInFlight = 2;

		vkInstance* m_Instance;
		vkDevice* m_Device;
	};
}