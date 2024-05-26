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

		const VkInstance& getInstance() { return m_Instance; }
	private:
		void initWindow();
		void createInstance();

	private:
		GLFWwindow* m_Window = nullptr;
		const int m_Width = 0;
		const int m_Height = 0;
		const std::string m_Title = "";

		VkInstance m_Instance;
	};
} // namespace karhu