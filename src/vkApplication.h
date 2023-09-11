#pragma once
//#include "vkInstance.h"
#include "vkDevice.h"
#include "vkInstance.h"
#include "vkSwapChain.h"
#include "vkPipeLine.h"
#include "vkdescriptors.h"
#include "model.h"
#include <iostream>
#include <vector>

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

namespace kge
{

	class vkApplication
	{
	public:
		vkApplication();
		vkApplication(const vkApplication&) = delete; /* Disallows copying. */
		vkApplication(vkApplication&&) = delete; /* Disallows move operation. */
		~vkApplication();

		vkApplication& operator=(const vkApplication&) = delete;
		vkApplication& operator=(vkApplication&&) = delete;

		void run();
		void drawFrame();

	private:
		/* Pointers to other classes. */
		//vkInstance* m_Instance = new vkInstance(enableValidationLayers);
		vkInstance* m_Instance = new vkInstance(enableValidationLayers);
		vkDevice* m_Device = new vkDevice(m_Instance, enableValidationLayers);
		vkSwapChain* m_SwapChain = new vkSwapChain(m_Instance, m_Device);
		vkPipeline* m_PipeLine;
		kgeModel* m_Model;
		vkDescriptorSetLayout* m_DescriptorLayout;
		/* Class members. */
		uint32_t m_CurrentFrame = 0;
		const int m_MaxFramesInFlight = 2;
		VkPipelineLayout m_PipelineLayout;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		VkDescriptorSetLayoutBinding ubobinding{};
		VkDescriptorSetLayout m_DescriptorSetLayout;


		/* Private functions. */
		void loadModels();
		void createPipeLineLayout();
		void createPipeLine();
		void createCommandBuffers();
		void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageindex);
		//void updateUniformBuffer(uint32_t currentFrame);
		void initVulkan();
	};
}
