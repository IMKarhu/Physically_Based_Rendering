#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

class VulkanRenderer;

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

	[[nodiscard]] GLFWwindow* getWindow() const;
	[[nodiscard]] const bool getFrameBufferResize();
	void setFrameBufferResize(bool size);
	
private:
	GLFWwindow* m_Window;
	const char* m_Title;
	std::uint32_t m_Width;
	std::uint32_t m_Height;
	bool m_FrameBufferResized = false; /* Handle to flag window resize. */

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
