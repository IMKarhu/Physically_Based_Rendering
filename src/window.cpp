#include "window.h"
#include <stdexcept>

Window::Window()
{
	
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::createWindow()
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize glfw!");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height), m_Title, nullptr, nullptr);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

bool Window::windowShouldCose()
{
	return glfwWindowShouldClose(m_Window);
}

void Window::setWidth(const uint32_t& width)
{
	m_Width = width;
}

void Window::setHeight(const uint32_t& height)
{
	m_Height = height;
}

void Window::setTitle(const char* title)
{
	m_Title = title;
}
