#include "SwapChain.hpp"

#include "Device.hpp"
#include "Window.hpp"

#include <algorithm>

namespace karhu
{
    SwapChain::SwapChain(Device& device, std::unique_ptr<Window>& window)
        : m_device(device)
        , m_window(window)
    {
        createSwapChain();
        createSwapChainImageViews();
    }

    SwapChain::~SwapChain()
    {
        printf("calling swapchain class destructor\n");
        for (auto imageView : m_swapChainImageViews)
        {
            vkDestroyImageView(m_device.lDevice(), imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_device.lDevice(), m_swapChain, nullptr);
    }

    void SwapChain::createSwapChain()
    {
        utils::SwapChainSupportDetails swapChainSupport = m_device.querySwapChainSupport(m_device.pDevice());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createinfo.surface = m_window->getSurface();
        createinfo.minImageCount = imageCount;
        createinfo.imageFormat = surfaceFormat.format;
        createinfo.imageColorSpace = surfaceFormat.colorSpace;
        createinfo.imageExtent = extent;
        createinfo.imageArrayLayers = 1;
        createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_device.findQueueFamilies(m_device.pDevice());
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createinfo.queueFamilyIndexCount = 2;
            createinfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createinfo.queueFamilyIndexCount = 0;
            createinfo.pQueueFamilyIndices = nullptr;
        }

        createinfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createinfo.presentMode = presentMode;
        createinfo.clipped = VK_TRUE;
        createinfo.oldSwapchain = VK_NULL_HANDLE;
        
        VK_CHECK(vkCreateSwapchainKHR(m_device.lDevice(), &createinfo, nullptr, &m_swapChain));
        
        VK_CHECK(vkGetSwapchainImagesKHR(m_device.lDevice(), m_swapChain, &imageCount, nullptr));
        m_swapChainImages.resize(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(m_device.lDevice(), m_swapChain, &imageCount, m_swapChainImages.data()));
        
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void SwapChain::createSwapChainImageViews()
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            m_swapChainImageViews[i] = createImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t layerCount)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = flags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = layerCount;

        VkImageView imageView;
        VK_CHECK(vkCreateImageView(m_device.lDevice(), &createInfo, nullptr, &imageView));

        return imageView;
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (auto& format : std::as_const(formats))
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }
        return formats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (auto& presentMode : std::as_const(presentModes))
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                printf("using VK_PRESENT_MODE_MAILBOX_KHR\n");
                return presentMode;
            }
        }
        printf("using VK_PRESENT_MODE_FIFO_KHR\n");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(m_window->getWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}
