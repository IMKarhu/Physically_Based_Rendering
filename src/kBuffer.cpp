#include "kBuffer.hpp"

namespace karhu
{
	void kBuffer::destroy()
	{
		vkDestroyBuffer(m_Device, m_Buffer, nullptr);
		vkFreeMemory(m_Device, m_BufferMemory, nullptr);
	}
}