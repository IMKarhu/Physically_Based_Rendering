#pragma once

#include "utils/utils.hpp"

#include <optional>
#include <set>

namespace karhu
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class Device
    {
        public:
            Device(const VkInstance& instance, const VkSurfaceKHR& surface);
            ~Device();

            Device(const Device&) = delete;
            void operator=(const Device&) = delete;
            Device(Device&&) = delete;
            Device& operator=(Device&&) = delete;

            const VkDevice& lDevice() const { return m_device; }
            const VkPhysicalDevice& pDevice() const { return m_physicalDevice; }

            void initDevice();
            void pickPhysicalDevice();
            void createLogicalDevice();
            bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
            utils::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);
            uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);
            VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                    VkImageTiling tiling,
                    VkFormatFeatureFlags features);
        private:
            VkDebugUtilsMessengerEXT m_debugMessenger;
            void setupDebugMessenger();
            VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
            void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
        private:
            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;
            VkQueue m_graphicsQueue = VK_NULL_HANDLE;
            VkQueue m_presentQueue = VK_NULL_HANDLE;

            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
} // karhu namespace
