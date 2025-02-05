#include "kCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace karhu
{
	void kCamera::setPerspective(float fovy, float aspect, float near, float far)
	{
		m_ProjectionMatrix = glm::perspective(fovy, aspect, near, far);
	}
	void kCamera::setView(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	{
		//m_ViewMatrix = glm::lookAt(position, direction, up);
		glm::mat4 camTranslation = glm::translate(glm::mat4(1.0f), m_CameraVars.m_Position);
		glm::mat4 camRotation = getRotationMatrix();
		m_ViewMatrix = glm::inverse(camTranslation * camRotation);
	}
	void kCamera::setyxzView(glm::vec3 position, glm::vec3 rotation)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		m_ViewMatrix = glm::mat4{ 1.f };
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, position);
		m_ViewMatrix[3][1] = -glm::dot(v, position);
		m_ViewMatrix[3][2] = -glm::dot(w, position);

		/*inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;*/
	}
	void kCamera::setModel(glm::mat4 model)
	{
		m_ModelMatrix = model;
	}
	glm::mat4 kCamera::getRotationMatrix()
	{
		glm::quat pitchRot = glm::angleAxis(m_CameraVars.m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yawRot = glm::angleAxis(m_CameraVars.m_yaw, glm::vec3(0.0f, -1.0f, 0.0f));

		return glm::toMat4(yawRot) * glm::toMat4(pitchRot);
	}
	void kCamera::update(float dt)
	{
		glm::mat4 cameraRot = getRotationMatrix();
		m_CameraVars.m_Position += glm::vec3(cameraRot * glm::vec4(m_CameraVars.m_Velocity * 10.5f * dt, 0.0f));
	}
}

