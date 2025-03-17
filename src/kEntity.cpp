#include "kEntity.hpp"
//#include "kBuffer.hpp"

namespace karhu
{
	
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
	VkDescriptorBufferInfo kEntity::getBufferInfo(int index)
	{
		return m_EntityUbo[index]->bufferInfo();
	}
	void kEntity::updateBuffer(int index)
	{
		EntityData data{};
		data.modelMatrix = getTransformMatrix();

		m_EntityUbo[index]->memcopy(&data, sizeof(EntityData), 0);
	}
}