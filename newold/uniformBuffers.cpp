#include "uniformBuffers.h"
#include <chrono>

namespace kge
{
    UniformBuffers::UniformBuffers(Command* command) : Buffer(command)
    {
    }
    void UniformBuffers::createUniformBuffers(const VkDevice& device, const VkPhysicalDevice& pdevice, const int maxframesinflight)
	{
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(maxframesinflight);
        m_UniformBuffersMemory.resize(maxframesinflight);
        m_UniformBuffersMapped.resize(maxframesinflight);

        for (size_t i = 0; i < maxframesinflight; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_UniformBuffers[i], m_UniformBuffersMemory[i], device, pdevice);

            vkMapMemory(device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
	}
	void UniformBuffers::updateUniformBuffers(uint32_t currentimage, const VkExtent2D& swapchainextent)
	{
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f); //glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); /* Rotate around Z axis. */
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); /* Look above at 45 degree angle. */
        /* 45 degree FOV. important to calculate aspect ratio using swapchain extent to consider window resizing. */
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchainextent.width / (float)swapchainextent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; /* a little hack to flip sign on the scaling factor of Y axis. Need to do this because glm was designed for OGL where Y coordinate of the clip is inverted.*/

        memcpy(m_UniformBuffersMapped[currentimage], &ubo, sizeof(ubo));
	}
}