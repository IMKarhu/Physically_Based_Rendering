#pragma once
#include "macros.hpp"
#include <vector>
#include <cstdint>
#include <cstring>

#ifndef NDEBUG
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

        bool checkValidationSupport()
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

    } // namespace vkUtils
} // namespace karhu
