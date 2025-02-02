#pragma once
#include "kModel.hpp"
#include "kBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace karhu
{
	class kEntity
	{
	public:

		kEntity(const kEntity&) = delete;
		void operator=(const kEntity&) = delete;
		kEntity(kEntity&&) = default;
		kEntity& operator=(kEntity&&) = default;

		static kEntity createEntity();

		glm::mat4 getTransformMatrix() { auto transform = glm::translate(glm::mat4(1.0f), m_Position);
										 transform = glm::rotate(transform, m_Rotation.y, { 0.0f, 0.1f, 0.0f });
										 transform = glm::rotate(transform, m_Rotation.x, { 0.0f, 0.1f, 0.0f });
										 transform = glm::rotate(transform, m_Rotation.z, { 0.0f, 0.1f, 0.0f });
										 transform = glm::scale(transform, m_Scale);
										 return transform; }
		void setModel(std::shared_ptr<kModel> model);
		std::shared_ptr<kModel> getModel() { return m_Model; }

		void setPosition(glm::vec3 position);
		void setRotation(glm::vec3 rotation);

		kBuffer m_UniformBuffer;
		VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };
	private:
		kEntity(uint32_t id) : m_Id(id){}
		uint32_t m_Id;
		std::shared_ptr<kModel> m_Model{};
		glm::vec3 m_Position{};
		glm::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 m_Rotation{};
		

	};
}