#pragma once
#include <vulkan/vulkan.hpp>
#include "device.h"
#include "swapChain.h"
#include "texture.h"

#include <fstream>
#include <vector>
#include "glm/glm.hpp"

class Device;
class SwapChain;
class Texture;



//const std::vector<Vertex> vertices = {
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
//const std::vector<uint16_t> indices = {
//	0, 1, 2, 2, 3, 0,
//	4, 5, 6, 6, 7, 4
//};

class GraphicsPipeLine
{
public:
	GraphicsPipeLine(Device* device, SwapChain* swapchain, Texture* texture);
	~GraphicsPipeLine();

	void createPipeLine(const VkDescriptorSetLayout& dslayout);
	void createRenderPass();
	void createFrameBuffers();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	[[nodiscard]] const VkRenderPass getRenderPass() { return m_RenderPass; }
	[[nodiscard]] const std::vector<VkFramebuffer> getFrameBuffers() { return m_SwapChainFramebuffers; }
	[[nodiscard]] const VkPipeline getPipeLine() { return m_GraphicsPipeline; }
	[[nodiscard]] const VkPipelineLayout getPipeLineLayout() const { return m_PipelineLayout; }

private:

	Device* m_Device;
	SwapChain* m_SwapChain;
	Texture* m_Texture;

	VkPipelineLayout m_PipelineLayout;
	VkRenderPass m_RenderPass;
	VkPipeline m_GraphicsPipeline;

	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

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