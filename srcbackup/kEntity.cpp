#include "kEntity.hpp"

namespace karhu
{
	kEntity kEntity::createEntity()
	{
		static uint32_t id = 0;
		return kEntity{ id++ };
	}
	void kEntity::updateBuffer()
	{
		ObjBuffer objBuffer{};
		objBuffer.model = getTransformMatrix();

		memcpy(m_Buffer->m_BufferMapped, &objBuffer, sizeof(objBuffer));
	}
	void kEntity::setModel(std::shared_ptr<kModel> model)
	{
		m_Model = model;
	}
	void kEntity::setPosition(glm::vec3 position)
	{
		m_Position = position;
	}
	void kEntity::setRotation(glm::vec3 rotation)
	{
		m_Rotation = rotation;
	}
	void kEntity::setScale(glm::vec3 scale)
	{
		m_Scale = scale;
	}
}