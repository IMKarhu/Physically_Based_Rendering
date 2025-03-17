#include "kCubeMapRenderSystem.hpp"
#include "../kDevice.hpp"
#include "../kSwapChain.hpp"
#include "../kGraphicsPipeline.hpp"

namespace karhu
{
	kCubeMapRenderSystem::kCubeMapRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain, kGraphicsPipeline& graphicspipeline)
		: m_Device(device)
		, m_SwapChain(swapchain)
		, m_GraphicsPipeline(graphicspipeline)
	{
	}

	kCubeMapRenderSystem::~kCubeMapRenderSystem()
	{

	}

	void kCubeMapRenderSystem::createGraphicsPipeline(VkDescriptorSetLayout SetLayout)
	{
		GraphicsPipelineStruct pipelineStruct{};
		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.pipelineLayoutInfo.pSetLayouts = &SetLayout;

		m_GraphicsPipeline.createPipeline(pipelineStruct, "../shaders/CubeMapVert.spv", "../shaders/CubeMapFrag.spv");
	}
}