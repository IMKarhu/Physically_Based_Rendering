#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

class Window
{
public:
	Window(const char* title, std::uint32_t width, std::uint32_t height);
	Window(const Window&) = delete; /* Disallows copying. */
	Window(Window&&) = delete; /* Disallows move operation. */
	~Window();

	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) = delete;

	void pollEvents();
	bool windowShouldClose();

	GLFWwindow* getWindow() const;
	

private:
	GLFWwindow* m_Window;
	const char* m_Title;
	std::uint32_t m_Width;
	std::uint32_t m_Height;
};