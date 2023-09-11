#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Buffer.h"

#include <vector>

namespace kge
{
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class UniformBuffers : public Buffer
	{
	public:
		UniformBuffers(Command* command);
		void createUniformBuffers(const VkDevice& device, const VkPhysicalDevice& pdevice, const int maxframesinflight);
		void updateUniformBuffers(uint32_t currentimage, const VkExtent2D& swapchainextent);

		[[nodiscard]] const std::vector<VkBuffer> getUniformBuffers() const { return m_UniformBuffers; }
	private:
		/* Class members. */
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;

		
	};
}