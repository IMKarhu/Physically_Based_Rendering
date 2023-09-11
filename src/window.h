#pragma once
#include <iostream>
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <string>

class Window
{
public:
	Window(const char* title, uint32_t width, uint32_t height);
	Window(const Window&) = delete; /* Disallows copying. */
	Window(Window&&) = delete; /* Disallows move operation. */

	virtual ~Window();

	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) = delete;

	void initWindow();
	void pollEvents();
	bool shouldClose();
	static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height);
	void createWindowSurface(VkSurfaceKHR surface, const VkInstance& instance);

	void setFrameBufferResize(bool size);

	[[nodiscard]] GLFWwindow* getWindow() const { return m_Window; }
	[[nodiscard]] const bool getFrameBufferResize() const { return m_FrameBufferResized; }
	[[nodiscard]] const uint32_t getWidth() const { return m_Width; }
	[[nodiscard]] const uint32_t getHeight() const { return m_Height; }

private:
	/* Class members. */
	GLFWwindow* m_Window = nullptr;
	const char* m_Title;
	uint32_t m_Width;
	uint32_t m_Height;
	bool m_FrameBufferResized = false;
};