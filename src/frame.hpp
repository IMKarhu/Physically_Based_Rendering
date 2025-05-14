#pragma once

#include "kEntity.hpp"
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace karhu
{
	struct Frame
	{
		uint32_t currentFrameIndex;
		VkCommandBuffer commandBuffer;
		VkDescriptorSet globalSet;
		std::vector<kEntity>& entities;
		std::vector<kEntity>& unrealEntities;
	};
}