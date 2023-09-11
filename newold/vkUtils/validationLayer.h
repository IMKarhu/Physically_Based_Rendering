#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace kge
{
	namespace vkUtils
	{
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		bool checkValidationSupport();
	}
}