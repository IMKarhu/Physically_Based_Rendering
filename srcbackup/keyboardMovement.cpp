#include "keyboardMovement.hpp"
#include "kCamera.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


namespace karhu
{
	void keyboardMovement::update(GLFWwindow* window, float dt, kCamera& camera, int width, int height)
	{	
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.m_CameraVars.m_Velocity.z = -1;
		}
		else
		{
			camera.m_CameraVars.m_Velocity.z = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera.m_CameraVars.m_Velocity.z = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.m_CameraVars.m_Velocity.x = -1;
		}
		else
		{
			camera.m_CameraVars.m_Velocity.x = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.m_CameraVars.m_Velocity.x = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			//camera.m_CameraVars.m_Velocity = upVec;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			//camera.m_CameraVars.m_Velocity = upVec;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			glfwSetCursorPos(window, width / 2, height / 2);

			
			camera.m_CameraVars.m_yaw += 0.005f * static_cast<float>(mouseX - width / 2);
			camera.m_CameraVars.m_Pitch -= 0.005f * static_cast<float>(mouseY - height / 2);

		}
	}
}

