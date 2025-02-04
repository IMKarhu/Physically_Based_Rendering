#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_TO_ZERO
#include <glm/glm.hpp>

namespace karhu
{
	class kCamera
	{
	public:
		void setPerspective(float fovy, float aspect, float near, float far);
		void setView(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		void setyxzView(glm::vec3 position, glm::vec3 rotation);
		void setModel(glm::mat4 model);

		const glm::mat4& getProjection() { return m_ProjectionMatrix; }
		const glm::mat4& getView() { return m_ViewMatrix; }
		const glm::mat4& getModel() { return m_ModelMatrix; }
	private:
		glm::mat4 m_ProjectionMatrix{ 1.0f };
		glm::mat4 m_ViewMatrix{ 1.0f };
		glm::mat4 m_ModelMatrix{ 1.0f };

	};
}