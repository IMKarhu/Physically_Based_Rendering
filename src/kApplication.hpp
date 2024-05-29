#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"

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

        void createSurface();
        VkSwapchainCreateInfoKHR fillSwapchainCI();
    private:
        std::unique_ptr<kWindow> m_Window;
        std::shared_ptr<Vulkan_Device> m_VkDevice;
        std::shared_ptr<Vulkan_SwapChain> m_VkSwapChain;

        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
    };
} // namespace karhu
