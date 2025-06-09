#include "device.hpp"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace karhu
{
    Device::Device(const VkInstance& instance, const VkSurfaceKHR& surface)
        : m_instance(instance)
        , m_surface(surface)
    {
        initDevice();
    }
    Device::~Device()
    {
        printf("calling Device class destructor\n");
        if (enableValidationLayers)
        {
            destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }
        vkDestroyDevice(m_device, nullptr);
    }

    void Device::initDevice()
    {
        setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void Device::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));
        
        if (deviceCount == 0)
        {
        	throw std::runtime_error("Failed to find GPU with a vulkan support!\n");
        }
        
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));
        
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                m_physicalDevice = device;
                break;
            }
        }
        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find suitable GPU!\n");
        }
    }

    void Device::createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> quecreateinfos;
        std::set<uint32_t> uniqueQueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        
        for (uint32_t queFamily : uniqueQueFamilies)
        {
            VkDeviceQueueCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            createinfo.queueFamilyIndex = queFamily;
            createinfo.queueCount = 1;
            float queuePriority = 1.0f; /* This needs to be between 0.0 - 1.0. */
            createinfo.pQueuePriorities = &queuePriority;
            quecreateinfos.push_back(createinfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{}; /* Leave this as vk_false for now, still need it for device creation. */
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        
        VkPhysicalDeviceSynchronization2Features sync2Features{};
        sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2Features.synchronization2 = VK_TRUE;
        
        VkDeviceCreateInfo deviceinfo{};
        deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(uniqueQueFamilies.size());
        deviceinfo.pQueueCreateInfos = quecreateinfos.data();
        deviceinfo.pEnabledFeatures = &deviceFeatures;
        deviceinfo.enabledExtensionCount = static_cast<uint32_t>(utils::deviceExtensions.size());
        deviceinfo.ppEnabledExtensionNames = utils::deviceExtensions.data();
        deviceinfo.pNext = &sync2Features;

        if (enableValidationLayers)
        {
            deviceinfo.enabledLayerCount = static_cast<uint32_t>(utils::validationLayers.size());
            deviceinfo.ppEnabledLayerNames = utils::validationLayers.data();
        }
        else
        {
            deviceinfo.enabledLayerCount = 0;
        }
        
        VK_CHECK(vkCreateDevice(m_physicalDevice, &deviceinfo, nullptr, &m_device));

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    bool Device::isDeviceSuitable(VkPhysicalDevice physicalDevice)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        
        bool extensionsSupported = utils::checkDeviceExtensionSupport(physicalDevice);
        
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            utils::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
        
        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice physicalDevice)
    {
        QueueFamilyIndices indices;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queFamily : queueFamilies)
        {
            if (queFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);
            
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

    utils::SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice physicalDevice)
    {
        utils::SwapChainSupportDetails details;

        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &details.capabilities));
        
        uint32_t formaCount;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formaCount, nullptr));
        
        if (formaCount != 0)
        {
            details.formats.resize(formaCount);
            VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formaCount, details.formats.data()));
        }
        
        uint32_t presentModeCount;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr));
        
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, details.presentModes.data()));
        }
        
        return details;
    }

    uint32_t Device::findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

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

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        throw std::runtime_error("Could not find supported format!\n");
    }


    void Device::setupDebugMessenger()
    {
        if (!enableValidationLayers)
        {
            return;
        }
        
        VkDebugUtilsMessengerCreateInfoEXT createinfo{};
        utils::populateDebugMessengerCreateInfo(createinfo);
        
        if (createDebugUtilsMessengerEXT(m_instance, &createinfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create debug messenger!\n");
        }
    }

    VkResult Device::createDebugUtilsMessengerEXT(VkInstance instance,
                                                  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDebugUtilsMessengerEXT* pDebugMessenger)
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

    void Device::destroyDebugUtilsMessengerEXT(VkInstance instance,
                                               VkDebugUtilsMessengerEXT debugMessenger,
                                               const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
} // karhu namespace
