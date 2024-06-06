#include "kWindow.hpp"

namespace karhu
{
	kWindow::kWindow(std::string title, const int width, const int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{
		initWindow();
		createInstance();
	}

	kWindow::~kWindow()
	{
		vkDestroyInstance(m_Instance, nullptr);
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	bool kWindow::shouldClose() { return glfwWindowShouldClose(m_Window); }

	void kWindow::pollEvents() { glfwPollEvents(); }

	void kWindow::getFrameBufferSize(GLFWwindow* window, int width, int height)
	{
		glfwGetFramebufferSize(window, &width, &height);
	}

	void kWindow::waitEvents()
	{
		glfwWaitEvents();
	}

	void kWindow::setResize(const bool& resized)
	{
		m_Resized = resized;
	}

	void kWindow::initWindow()
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, frameBufferResizeCallback);
	}

	void kWindow::createInstance()
	{
		if (enableValidationLayers && !vkUtils::checkValidationSupport())
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

		auto extensions = vkUtils::getRequiredExtensions();
		createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createinfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
		if (enableValidationLayers)
		{
			createinfo.enabledLayerCount = static_cast<uint32_t>(vkUtils::valiadationLayers.size());
			createinfo.ppEnabledLayerNames = vkUtils::valiadationLayers.data();
			vkUtils::populateDebugMessengerCreateInfo(debuginfo);
			createinfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debuginfo;
		}
		else
		{
			createinfo.enabledLayerCount = 0;
			createinfo.pNext = nullptr;
		}

		/*if (vkCreateInstance(&createinfo, nullptr, &m_Instance) != VK_SUCCESS)
		{
			std::runtime_error("Failed to create Instance!");
		}*/
		VK_CHECK(vkCreateInstance(&createinfo, nullptr, &m_Instance));
	}
	void kWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<kWindow*>(glfwGetWindowUserPointer(window));
		app->m_Resized = true;
	}
} // namespace karhu