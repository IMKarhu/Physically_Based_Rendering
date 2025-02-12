#include "keyboardMovement.hpp"
//#include "kEntity.hpp"
#include "kCamera.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


namespace karhu
{
	void keyboardMovement::update(GLFWwindow* window, float dt, kCamera& camera, int width, int height)
	{
		//glm::vec3 rotate{ 0 };
		/*if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		{
			rotate.y -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		{
			rotate.y += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		{
			rotate.x += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{
			rotate.x -= 1.0f;
		}*/

		/*if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		{
			glm::vec3 tempRotation = entity.getRotation();
			tempRotation += m_RotateSpeed * dt * glm::normalize(rotate);
			entity.setRotation(tempRotation);
		}

		entity.setRotation({ glm::clamp(entity.getRotation().x, -1.5f, 1.5f),
							 glm::mod(entity.getRotation().y, glm::two_pi<float>()),
							 entity.getRotation().z });*/


		//float yaw = entity.getRotation().y;
		//float pitch = entity.getRotation().z;
		//glm::vec3 forwardVec{ glm::sin(yaw), 0.0f, glm::cos(yaw)};
		////glm::vec3 forwardVec{ glm::cos(pitch)*glm::cos(yaw), -glm::sin(pitch), glm::sin(yaw)*glm::cos(pitch)};
		//const glm::vec3 upVec{ 0.0f, -1.0f, 0.0f };
		//const glm::vec3 rightVec{ forwardVec.z, 0.0f, -forwardVec.x };
		//const glm::vec3 rightVec = glm::cross(glm::normalize(forwardVec), glm::normalize(upVec));
		
		//glm::vec3 moveDirection{ 0.0f };
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


			/*if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				float z = entity.getRotation().z;
				entity.setRotation({ 0.0f, rotate.y, fminf(89.0f, fmaxf(-89.0f, z + z)) });
			}*/
		}

		//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
		//{
		//	camera.m_CameraVars.m_Velocity.z = 0;
		//}
		//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
		//{
		//	camera.m_CameraVars.m_Velocity.z = 0;
		//}
		//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
		//{
		//	camera.m_CameraVars.m_Velocity.x = 0;
		//}
		//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
		//{
		//	camera.m_CameraVars.m_Velocity.x = 0;
		//}
		//if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
		//{
		//	//camera.m_CameraVars.m_Velocity = upVec;
		//}
		//if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
		//{
		//	//camera.m_CameraVars.m_Velocity = upVec;
		//}

		/*if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
		{
			glm::vec3 tempPosition = entity.getPosition();
			tempPosition += m_Speed * dt * glm::normalize(moveDirection);
			entity.setPosition(tempPosition);
			
		}*/

		

		/*glm::vec3 tempRotation = entity.getRotation() * m_Speed * dt * glm::normalize(rotate);
		entity.setRotation(tempRotation);*/
		/*double mouseX, mouseY;
		//glfwGetCursorPos(window, &mouseX, &mouseY);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			glfwSetCursorPos(window, width / 2, height / 2);
			rotate.y = 0.1f * static_cast<float>(mouseX - width / 2);
			rotate.z = 0.1f * static_cast<float>(mouseY - height / 2);


			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				float z = entity.getRotation().z;
				entity.setRotation({ 0.0f, rotate.y, fminf(89.0f, fmaxf(-89.0f, z + z)) });
			}
		}*/
	}
}

