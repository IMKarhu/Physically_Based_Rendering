#pragma once
#include <iostream>
#include "GLFW/glfw3.h"
#include <string>

class Window
{
public:
	Window(const char* title, uint32_t width, uint32_t height);
	~Window();

	void initWindow();
	void pollEvents();
	bool shouldClose();
	static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height);

	void setFrameBufferResize(bool size);

	[[nodiscard]] GLFWwindow* getWindow() const { return m_Window; }
	[[nodiscard]] const bool getFrameBufferResize() const { return m_FrameBufferResized; }

private:
	/* Class members. */
	GLFWwindow* m_Window = nullptr;
	const char* m_Title;
	uint32_t m_Width;
	uint32_t m_Height;
	bool m_FrameBufferResized = false;
};