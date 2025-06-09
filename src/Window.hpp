#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

namespace karhu
{
    class Window
    {
        public:
            Window(std::string title, const int width, const int height);
            ~Window();

            Window(const Window&&) = delete;
            Window& operator=(const Window&) = delete;

            bool windowShouldClose();
            void pollEvents();
            void waitEvents();

            const VkInstance& getInstance();
            VkSurfaceKHR getSurface();
            GLFWwindow* getWindow();
        private:
            void initWindow();
            void createInstance();
            void createSurface();
            static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height);
        private:
            GLFWwindow* m_window = nullptr;
            const int m_width = 0;
            const int m_height = 0;
            const std::string m_title = "";
            bool m_resized = false;

            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
} // karhu namespace
