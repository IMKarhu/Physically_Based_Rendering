#include "kCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace karhu
{
	void kCamera::setPerspective(float fovy, float aspect, float near, float far)
	{
		m_ProjectionMatrix = glm::perspective(fovy, aspect, near, far);
	}
	void kCamera::setView(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	{
		m_ViewMatrix = glm::lookAt(position, direction, up);
	}
	void kCamera::setModel(glm::mat4 model, float angle, glm::vec3 axis)
	{
		m_ModelMatrix = glm::rotate(model, angle, axis);
	}
}

