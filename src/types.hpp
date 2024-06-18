#pragma once

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>


namespace karhu
{


    struct UniformBufferObject
    {
        alignas(16)glm::mat4 model;
        alignas(16)glm::mat4 view;
        alignas(16)glm::mat4 proj;
    };

    class vkglTFModel
    {
    public:

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;

            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription description{};
                description.binding = 0;
                description.stride = sizeof(Vertex);
                description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return description;
            }
            static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription()
            {
                std::array<VkVertexInputAttributeDescription, 2> attributeDescription{};
                attributeDescription[0].binding = 0;
                attributeDescription[0].location = 0;
                attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescription[0].offset = offsetof(Vertex, pos);

                attributeDescription[1].binding = 0;
                attributeDescription[1].location = 1;
                attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescription[1].offset = offsetof(Vertex, color);

                return attributeDescription;
            }
        };

        struct
        {
            VkBuffer m_Buffer;
            VkDeviceMemory m_BufferMemory;
        }m_Vertices;

        struct
        {
            VkBuffer m_Buffer;
            VkDeviceMemory m_BufferMemory;
        }m_Indices;

    private:
    };
}