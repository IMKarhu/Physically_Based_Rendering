#include "window.h"
#include <stdexcept>

Window::Window(const char* title, std::uint32_t width, std::uint32_t height)
	:m_Title(title),
	m_Width(width),
	m_Height(height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); /* Tell glfw to not create opengl context. */

	m_Window = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height), m_Title, nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::pollEvents()
{
	glfwPollEvents();
}

bool Window::windowShouldClose()
{
	return glfwWindowShouldClose(m_Window);
}

GLFWwindow* Window::getWindow() const
{
	return m_Window;
}

const bool Window::getFrameBufferResize()
{
	return m_FrameBufferResized;
}

void Window::setFrameBufferResize(bool size)
{
	m_FrameBufferResized = size;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	app->m_FrameBufferResized = true;
}
