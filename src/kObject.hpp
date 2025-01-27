#pragma once
#include <vulkan/vulkan.h>


namespace karhu
{
	class Model
	{
	public:
		Model();
		~Model();

		struct
		{
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
		}m_VertexBuffer;

		struct
		{
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
		}m_IndexBuffer;

	private:
	};
}