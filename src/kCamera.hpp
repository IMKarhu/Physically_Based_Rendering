#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <chrono>

namespace karhu
{
	class Camera
	{
	public:
		Camera(GLFWwindow *window, uint32_t width, uint32_t height);
		~Camera();

		void setMatrices();

		void update(float deltaTime);
		const glm::mat4& perspectiveMat() const { return glm::perspective(glm::radians(45.0f), m_Width / (float)m_Height, 0.1f, 100.0f); }
		const glm::mat4& viewMat() const { return glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), m_Position, m_Up); }
		const glm::mat4& modelMat() const { return glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); }
	private:
		uint32_t m_Width;
		uint32_t m_Height;
		float time = 0.0f;
		float m_CameraSpeed = 0.05f;

		glm::vec3 m_Position;
		glm::vec3 m_Up;
		glm::vec3 m_Forward;
		glm::vec3 m_Right;
		glm::vec3 m_Eulers;

		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ModelMatrix;
		glm::mat4 m_Perspectivematrix;

		GLFWwindow* m_Window;
	};
} // karhu