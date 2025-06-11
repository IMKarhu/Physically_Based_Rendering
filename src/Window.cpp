#include "Window.hpp"

#include "utils/utils.hpp"

namespace karhu
{
    Window::Window(std::string title, const int width, const int height)
        : m_title(title)
        , m_width(width)
        , m_height(height)
    {
        initWindow();
        createInstance();
        createSurface();
    }

    Window::~Window()
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    bool Window::windowShouldClose()
    {
        return glfwWindowShouldClose(m_window);
    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }

    void Window::waitEvents()
    {
        glfwWaitEvents();
    }

    void Window::frameBufferSize(GLFWwindow* window, int& width, int& height)
    {
        glfwGetFramebufferSize(window, &width, &height);
    }

    void Window::setResized(bool resized)
    {
        m_resized = resized;
    }

    const VkInstance& Window::getInstance()
    {
        return m_instance;
    }

    VkSurfaceKHR Window::getSurface()
    {
        return m_surface;
    }

    GLFWwindow* Window::getWindow()
    {
        return m_window;
    }

    bool Window::resized()
    {
        return m_resized;
    }

    void Window::initWindow()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW!");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallBack);
    }

    void Window::createInstance()
    {
        if (enableValidationLayers && !utils::checkValidationSupport())
        {
            throw std::runtime_error("No available validation layers!");
        }
        
        VkApplicationInfo appinfo{};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.pApplicationName = "Small vulkan renderer";
        appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appinfo.pEngineName = "LightningStrike";
        appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appinfo.apiVersion = VK_API_VERSION_1_3;
        
        VkInstanceCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createinfo.pApplicationInfo = &appinfo;
        
        auto extensions = utils::getRequiredExtensions();
        createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createinfo.ppEnabledExtensionNames = extensions.data();
        
        VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
        if (enableValidationLayers)
        {
            createinfo.enabledLayerCount = static_cast<uint32_t>(utils::validationLayers.size());
            createinfo.ppEnabledLayerNames = utils::validationLayers.data();
            utils::populateDebugMessengerCreateInfo(debuginfo);
            createinfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debuginfo;
        }
        else
        {
            createinfo.enabledLayerCount = 0;
            createinfo.pNext = nullptr;
        }
        
        if (vkCreateInstance(&createinfo, nullptr, &m_instance) == VK_SUCCESS)
        {
            std::cout << "Instance succesfully created!\n" << std::endl;
        }
    }

    void Window::createSurface()
    {
        glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
    }

    void Window::frameBufferResizeCallBack(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        app->m_resized = true;
    }
} // karhu namespace
