#include "queuefamilies.h"

namespace kge
{
	namespace vkUtils
	{
		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& pdevice, const VkSurfaceKHR& surface)
		{
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.graphicsFamily = i;
                }
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, surface, &presentSupport);
                if (presentSupport)
                {
                    indices.presentFamily = i;
                }
                if (indices.isComplete())
                {
                    break;
                }

                i++;
            }

            return indices;
		}
	}
}