#pragma once
#include "vkSwapChain.h"
#include "device.h"
#include "model/model.h"
#include "vulkan/vulkan.h"
#include <iostream>
#include <fstream>
#include <vector>

namespace kge
{
//	const std::vector<Vertex> vertices = {
//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // pos, color, texCoords
//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // pos, color, texCoords
//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // pos, color, texCoords
//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // pos, color, texCoords
//
//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // pos, color, texCoords
//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // pos, color, texCoords
//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // pos, color, texCoords
//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}} // pos, color, texCoords
//};
//
//	const std::vector<uint16_t> indices = {
//		0, 1, 2, 2, 3, 0,
//		4, 5, 6, 6, 7, 4
//	};

	//class vkSwapChain;

	class VulkanPipeLine
	{
	public:
		VulkanPipeLine(vkSwapChain* swapchain, Device* device, Model* model);
		~VulkanPipeLine();

		void createPipeLine(const VkDescriptorSetLayout& dslayout);
		void createRenderPass();
		void createFrameBuffers();

		[[nodiscard]] const VkPipelineLayout getPipeLineLayout() const { return m_PipelineLayout; }
		[[nodiscard]] const VkPipeline getPipeLine() const { return m_GraphicsPipeline; }
		[[nodiscard]] const VkRenderPass getRenderPass() const { return m_RenderPass; }
		[[nodiscard]] const std::vector<VkFramebuffer> getSCFrameBuffers() const { return m_SwapChainFramebuffers; }

	private:
		/*Class members. */
		VkPipelineLayout m_PipelineLayout;
		VkRenderPass m_RenderPass;
		VkPipeline m_GraphicsPipeline;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		vkSwapChain* m_SwapChain;
		Device* m_Device;
		Model* m_Model;

		/* Private functions. */
		VkShaderModule createShaderModule(const std::vector<char>& code);

		static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}
			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}
	};
}