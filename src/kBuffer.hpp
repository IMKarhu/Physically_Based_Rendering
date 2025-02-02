#pragma once
#include <vulkan/vulkan.h>

namespace karhu
{
	struct kBuffer
	{
		VkDevice m_Device;
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		void* m_BufferMapped;
		void destroy();
	};
}