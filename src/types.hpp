#pragma once

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>


namespace karhu
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texcoords;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription description{};
            description.binding = 0;
            description.stride = sizeof(Vertex);
            description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return description;
        }
        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescription{};
            attributeDescription[0].binding = 0;
            attributeDescription[0].location = 0;
            attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescription[0].offset = offsetof(Vertex, pos);

            attributeDescription[1].binding = 0;
            attributeDescription[1].location = 1;
            attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescription[1].offset = offsetof(Vertex, color);

            attributeDescription[2].binding = 0;
            attributeDescription[2].location = 2;
            attributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescription[2].offset = offsetof(Vertex, normal);

            attributeDescription[3].binding = 0;
            attributeDescription[3].location = 3;
            attributeDescription[3].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescription[3].offset = offsetof(Vertex, texcoords);

            return attributeDescription;
        }
    };

    struct UniformBufferObject
    {
        glm::mat4 view;
        glm::mat4 proj;
    };

    // struct ObjBuffer
    // {
    //     glm::mat4 model;
    // };

    struct ObjPushConstant
    {
        glm::mat4 model;
    };

    struct pushConstants
    {
        glm::vec3 cameraPosition;
        int offset;
        alignas(16) glm::vec3 lightPosition;
        glm::vec4 lighColor;
        glm::vec4 albedoNormalMetalRoughness;
    };
}
