#pragma once

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>


namespace karhu
{
    class Device;
    class Window;

    class SwapChain
    {
        public:
            SwapChain(Device& device, std::unique_ptr<Window>& window);
            ~SwapChain();

            SwapChain(const SwapChain&) = delete;
            void operator=(const SwapChain&) = delete;
            SwapChain(SwapChain&&) = delete;
            SwapChain& operator=(SwapChain&&) = delete;

            void createSwapChain();
            void createSwapChainImageViews();
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t layerCount);

            VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
            VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

            const VkFormat& getSwapChainImageFormat() const { return m_swapChainImageFormat; }
            const std::vector<VkImageView>& getSwapChainImageviews() const { return m_swapChainImageViews; }
            const VkExtent2D& getSwapChainExtent() const { return m_swapChainExtent; }
        private:
            VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
            std::vector<VkImage> m_swapChainImages;
            VkFormat m_swapChainImageFormat;
            VkExtent2D m_swapChainExtent;
            std::vector<VkImageView> m_swapChainImageViews;


            std::unique_ptr<Window>& m_window;
            Device& m_device;
    };
} // karhu namespace
