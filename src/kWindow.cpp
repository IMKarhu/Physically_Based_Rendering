#include "kWindow.hpp"

namespace karhu
{
	kWindow::kWindow(std::string title, const int width, const int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{
		initWindow();
	}
	kWindow::~kWindow()
	{
	}
	bool kWindow::shouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}
	void kWindow::pollEvents()
	{
		glfwPollEvents();
	}
	void kWindow::initWindow()
	{
		if (!glfwInit())
		{
			std::runtime_error("Failed to initialize GLFW!");
		}
		
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	}
} // karhu namespace