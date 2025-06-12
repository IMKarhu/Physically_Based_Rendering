#pragma once

#include "macros.hpp"
#include "GLFW/glfw3.h"

#include <vector>
#include <cstdint>
#include <cstring>
#include <set>


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace karhu
{
    namespace utils
    {
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
        };

        inline bool checkValidationSupport()
        {
            uint32_t layerCount;
            VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

            std::vector<VkLayerProperties> availabeLayers(layerCount);
            VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availabeLayers.data()));

            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;
                for (const auto& layerProperties : availabeLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }
                if (!layerFound)
                {
                    return false;
                }
            }

            return true;
        }

        inline std::vector<const char*> getRequiredExtensions()
        {
            uint32_t extensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

            if (enableValidationLayers)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }

        inline void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createinfo)
        {
            createinfo = {};
            createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT This could be useful for debugging? see docs for more information. */
            createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createinfo.pfnUserCallback = debugCallback;
            createinfo.pUserData = nullptr;
        }

        inline bool checkDeviceExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t deviceExtensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(deviceExtensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            for (const auto& extension : availableExtensions)
            {
                requiredExtensions.erase(extension.extensionName);
            }
            return requiredExtensions.empty();
        }

        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t filter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (size_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            throw std::runtime_error("could not find memory type!\n");
            return 0;
        }


        inline VkDescriptorSetLayoutBinding fillLayoutBindingStruct(uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags flags)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorType = type;
            layoutBinding.descriptorCount = descriptorCount;
            layoutBinding.stageFlags = flags;
            layoutBinding.pImmutableSamplers = nullptr;
            return layoutBinding;
        }

        inline VkDescriptorSetLayoutCreateInfo fillDescriptorSetLayoutCreateInfo()
        {
            VkDescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            return createInfo;
        }
        inline void createBuffers(VkDevice device,
                VkPhysicalDevice physicalDevice,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkBuffer& buffer,
                VkDeviceMemory& bufferMemory)
        {
            VkBufferCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createinfo.size = size; //byte size of one vertices multiplied by size of vector
            createinfo.usage = usage;
            createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            
            VK_CHECK(vkCreateBuffer(device, &createinfo, nullptr, &buffer));
            
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
            
            VkMemoryAllocateInfo allocinfo{};
            allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocinfo.allocationSize = memRequirements.size;
            allocinfo.memoryTypeIndex = utils::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            
            VK_CHECK(vkAllocateMemory(device, &allocinfo, nullptr, &bufferMemory));
            VK_CHECK(vkBindBufferMemory(device, buffer, bufferMemory, 0));

        }

    } // utils namespace
} // karhu namespace
