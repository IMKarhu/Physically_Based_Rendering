#include "kApplication.hpp"


namespace karhu
{
    Application::Application()
    {
        m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
        
    }

    Application::~Application()
    {
        
        printf("app destroyed\n");
        vkDestroySwapchainKHR(m_VkDevice->m_Device, m_VkSwapChain->m_SwapChain, nullptr);
        vkDestroySurfaceKHR(m_Window->getInstance(), m_Surface, nullptr);
        if (enableValidationLayers)
        {
            destroyDebugUtilsMessengerEXT(m_Window->getInstance(), m_DebugMessenger, nullptr);
        }
    }

    void Application::run()
    {
        setupDebugMessenger();
        createSurface();
        m_VkDevice = std::make_shared<Vulkan_Device>(m_Window->getInstance(), m_Surface);
        m_VkDevice->pickPhysicalDevice();
        m_VkDevice->createLogicalDevice();
        VkSwapchainCreateInfoKHR createinfo = fillSwapchainCI();
        //SwapChainSupportDetails details = m_VkDevice.querySwapChainSupport(m_VkDevice.m_PhysicalDevice);
        printf("before swapchain creation.");
        m_VkSwapChain = std::make_shared<Vulkan_SwapChain>(m_VkDevice->m_Device, m_Window->getWindow());
        m_VkSwapChain->createSwapChain(m_VkDevice->querySwapChainSupport(m_VkDevice->m_PhysicalDevice), m_Surface, createinfo);
        m_VkSwapChain->createImageViews();

        while (!m_Window->shouldClose())
        {
            m_Window->pollEvents();
        }
    }

    void Application::setupDebugMessenger()
    {
        if (!enableValidationLayers)
        {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createinfo{};
        vkUtils::populateDebugMessengerCreateInfo(createinfo);
        
        if (createDebugUtilsMessengerEXT(m_Window->getInstance(), &createinfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create debug messenger!\n");
        }
    }

    VkResult Application::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Application::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void Application::createSurface()
    {
        VK_CHECK(glfwCreateWindowSurface(m_Window->getInstance(), m_Window->getWindow(), nullptr, &m_Surface));
    }

    VkSwapchainCreateInfoKHR Application::fillSwapchainCI()
    {
        VkSwapchainCreateInfoKHR createinfo{};
        QueueFamilyIndices indices = m_VkDevice->findQueueFamilies(m_VkDevice->m_PhysicalDevice);

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
        return createinfo;
    }

    

} // namespace karhu
