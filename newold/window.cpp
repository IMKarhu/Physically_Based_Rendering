#include "window.h"


Window::Window(const char* title, uint32_t width, uint32_t height)
	:m_Title(title), m_Width(width), m_Height(height)
{
	initWindow();
}

Window::~Window()
{
	printf("hello window");
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); /* Tell glfw to not create opengl context. */

	m_Window = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height), m_Title, nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, frameBufferResizeCallBack);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(m_Window);
}

void Window::frameBufferResizeCallBack(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	app->m_FrameBufferResized = true;
}

void Window::setFrameBufferResize(bool size)
{
	m_FrameBufferResized = size;
}
