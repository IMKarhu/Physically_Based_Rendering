#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"

#include <memory>

namespace karhu
{
    class Application
    {
    public:
        Application();
        ~Application();
        void run();
    private:
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    private:
        std::unique_ptr<kWindow> m_Window;
        Vulkan_Device m_VkDevice;

        VkDebugUtilsMessengerEXT m_DebugMessenger;
    };
} // namespace karhu
