#pragma once
#include "macros.hpp"
#include "GLFW/glfw3.h"
#include <vector>
#include <cstdint>
#include <cstring>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace karhu
{
    namespace vkUtils
    {
        const std::vector<const char*> valiadationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        inline bool checkValidationSupport()
        {
            uint32_t layerCount;
            VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

            std::vector<VkLayerProperties> availabeLayers(layerCount);
            VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availabeLayers.data()));

            for (const char* layerName : valiadationLayers)
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
            createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createinfo.pfnUserCallback = vkUtils::debugCallback;
            createinfo.pUserData = nullptr;
        }

    } // namespace vkUtils
} // namespace karhu
