#include "vkInstance.h"
#include "vkUtils/validationLayer.h"
#include "vkUtils/debugUtilsMessenger.h"

namespace kge
{
    vkInstance::vkInstance(const bool& enableValidationLayers) : Window("Vulkan PBR renderer", 800, 600)
    {
        if (enableValidationLayers && !vkUtils::checkValidationSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "PBR Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "KarhuGraphicsEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = debugUtils::getRequiredExtensions(enableValidationLayers);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(vkUtils::validationLayers.size());
            createInfo.ppEnabledLayerNames = vkUtils::validationLayers.data();

            debugUtils::populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        setupDebugMessenger(enableValidationLayers);
        createSurface();
    }
    vkInstance::~vkInstance()
    {
        std::cout << "Instance destructor" << std::endl;
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void vkInstance::cleanUp(const bool& enableValidationLayers)
    {
        if (enableValidationLayers) {
            debugUtils::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }
    }
    
    void vkInstance::setupDebugMessenger(const bool& enableValidationLayers)
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        debugUtils::populateDebugMessengerCreateInfo(createInfo);

        if (debugUtils::CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void vkInstance::createSurface()
    {
        if (glfwCreateWindowSurface(m_Instance, getWindow(), nullptr, &m_Surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}

