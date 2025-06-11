#include "Entity.hpp"

namespace karhu
{
    Entity Entity::createEntity()
    {
        static uint32_t id = 0;
        return Entity{ id++ };
    }
    void Entity::updateBuffer()
    {
        ObjBuffer objBuffer{};
        objBuffer.model = getTransformMatrix();
        
        memcpy(m_Buffer->m_bufferMapped, &objBuffer, sizeof(objBuffer));
    }
    void Entity::setModel(std::shared_ptr<Model> model)
    {
        m_Model = model;
    }
    void Entity::setPosition(glm::vec3 position)
    {
        m_Position = position;
    }
    void Entity::setRotation(glm::vec3 rotation)
    {
        m_Rotation = rotation;
    }
    void Entity::setScale(glm::vec3 scale)
    {
        m_Scale = scale;
    }
}
