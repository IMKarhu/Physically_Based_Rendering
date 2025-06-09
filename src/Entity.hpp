#pragma once
#include "kModel.hpp"
#include "Buffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace karhu
{
    class Entity
    {
    public:
        Entity(const Entity&) = delete;
        void operator=(const Entity&) = delete;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;
        
        static Entity createEntity();
        static Entity createLight();
        
        void updateBuffer();
        
        glm::mat4 getTransformMatrix() { auto transform = glm::translate(glm::mat4(1.0f), m_Position);
            transform = glm::rotate(transform, m_Rotation.x, { 1.0f, 0.0f, 0.0f });
            transform = glm::rotate(transform, m_Rotation.y, { 0.0f, 1.0f, 0.0f });
            transform = glm::rotate(transform, m_Rotation.z, { 0.0f, 0.0f, 1.0f });
            transform = glm::scale(transform, m_Scale);
            return transform; }
        void setModel(std::shared_ptr<kModel> model);
        std::shared_ptr<kModel> getModel() { return m_Model; }
        
        void setPosition(glm::vec3 position);
        glm::vec3 getPosition() { return m_Position; }
        void setRotation(glm::vec3 rotation);
        glm::vec3& getRotation() { return m_Rotation; }
        void setScale(glm::vec3 scale);
        glm::vec3& getScale() { return m_Scale; }
        uint32_t getId() { return m_Id; }
        
        VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };
        std::unique_ptr<Buffer> m_Buffer;
    private:
        Entity(uint32_t id) : m_Id(id){}
        uint32_t m_Id;
        uint32_t mLightId;
        std::shared_ptr<kModel> m_Model{};
        glm::vec3 m_Position{};
        glm::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };
        glm::vec3 m_Rotation{};
        
    
    };
}
