#pragma once
#include <vulkan/vulkan.hpp>
#include "device.h"
#include "swapChain.h"

#include <fstream>
#include <vector>

class Device;
class SwapChain;

class GraphicsPipeLine
{
public:
	GraphicsPipeLine(Device* device, SwapChain* swapchain);
	~GraphicsPipeLine();

	void createPipeLine();
	void createRenderPass();
	void createFrameBuffers();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	[[nodiscard]] const VkRenderPass getRenderPass() { return m_RenderPass; }
	[[nodiscard]] const std::vector<VkFramebuffer> getFrameBuffers() { return m_SwapChainFramebuffers; }
	[[nodiscard]] const VkPipeline getPipeLine() { return m_GraphicsPipeline; }
private:

	Device* m_Device;
	SwapChain* m_SwapChain;

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