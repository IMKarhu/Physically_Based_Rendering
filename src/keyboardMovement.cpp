#include "keyboardMovement.hpp"
#include "kEntity.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


namespace karhu
{
	void keyboardMovement::update(GLFWwindow* window, float dt, kEntity& entity, int width, int height)
	{
		glm::vec3 rotate{ 0 };
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
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
		}
		float yaw = entity.getRotation().y;
		float pitch = entity.getRotation().z;
		//glm::vec3 forwardVec{ glm::sin(yaw), 0.0f, glm::cos(yaw)};
		glm::vec3 forwardVec{ glm::cos(pitch)*glm::cos(yaw), -glm::sin(pitch), glm::sin(yaw)*glm::cos(pitch)};
		const glm::vec3 upVec{ 0.0f, 1.0f, 0.0f };
		//const glm::vec3 rightVec{ forwardVec.z, 0.0f, -forwardVec.x };
		const glm::vec3 rightVec = glm::cross(glm::normalize(forwardVec), glm::normalize(upVec));
		
		glm::vec3 moveDirection{ 0.0f };
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			//printf("forwardvec: \n", glm::to_string(forwardVec));
			moveDirection += forwardVec;
			//printf("direction: ", glm::to_string(moveDirection));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			moveDirection -= forwardVec;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			moveDirection += rightVec;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			moveDirection -= rightVec;
		}

		if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
		{
			glm::vec3 tempPosition = entity.getPosition();
			tempPosition += m_Speed * dt * glm::normalize(moveDirection);
			//printf("%f %f %f \n", glm::to_string(tempPosition));
			//tempPosition += m_Speed * dt * glm::normalize(moveDirection);;
			entity.setPosition(tempPosition);
		}

		

		/*glm::vec3 tempRotation = entity.getRotation() * m_Speed * dt * glm::normalize(rotate);
		entity.setRotation(tempRotation);*/
	}
}

