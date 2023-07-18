#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class Window
{
public:
	Window();
	Window(const Window&) = delete; /* Disallows copying. */
	Window(Window&&) = delete; /* Disallows move operation. */
	~Window();

	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) = delete;

	void createWindow();
	void pollEvents();
	bool windowShouldCose();

	GLFWwindow* getWindow();

	void setWidth(const uint32_t &width);
	void setHeight(const uint32_t &height);
	void setTitle(const char* title);
private:
	GLFWwindow* m_Window = nullptr;
	const char* m_Title;
	std::uint32_t m_Width;
	std::uint32_t m_Height;
};