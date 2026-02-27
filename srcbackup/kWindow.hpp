#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "utils/vkUtils.hpp"


#include <stdexcept>
#include <string>

namespace karhu
{
	class kWindow
	{
	public:
		kWindow(std::string title, const int width, const int height);
		~kWindow();

		kWindow(const kWindow&&) = delete;
		kWindow& operator=(const kWindow&) = delete;

		bool shouldClose();
		void pollEvents();
		void getFrameBufferSize(GLFWwindow* window, int width, int height);
		void waitEvents();

		const VkInstance& getInstance() { return m_Instance; }
		VkSurfaceKHR getSurface() { return m_Surface; }
		GLFWwindow* getWindow() { return m_Window; }
		const int getWidth() const { return m_Width; }
		const int getheight() const { return m_Height; }
		const bool& getResize() { return m_Resized; }
		void setResize(const bool& resized);
	private:
		void initWindow();
		void createInstance();
		void createSurface();
		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
	private:
		GLFWwindow* m_Window = nullptr;
		const int m_Width = 0;
		const int m_Height = 0;
		const std::string m_Title = "";
		bool m_Resized = false;

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
	};
} // namespace karhu