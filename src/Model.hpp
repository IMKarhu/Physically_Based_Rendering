#pragma once
#include "Device.hpp"
#include "types.hpp"
#include "Texture.hpp"

#include <assimp/scene.h>

namespace karhu
{

    class CommandBuffer;
    class Model
    {
        public:
            Model(Device& device, CommandBuffer& commandBuffer, std::string filepath);
            Model(Device& device, CommandBuffer& commandBuffer, std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool hdr = false);
            ~Model();
            
            Model(const Model&) = delete;
            void operator=(const Model&) = delete;
            
            struct {
                VkBuffer m_vertexBuffer;
                VkDeviceMemory m_vertexBufferMemory;
            }m_vertexBuffer;
            
            struct {
                VkBuffer m_indexBuffer;
                VkDeviceMemory m_indexBufferMemory;
            }m_indexBuffer;
            
            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);
            void loadModel(std::string filepath);
            void processScene(const aiScene* scene);
            void processNode(aiMesh* mesh, const aiScene* scene);
            
            std::vector<uint32_t>& getIndices() { return m_indices; }
            void createVertexBuffer();
            void createVertexBuffer(std::vector<Vertex>& vertices);
            void createIndexBuffer();
            void createIndexBuffer(std::vector<uint32_t>& indices);
            std::vector<NTexture> m_Textures;
        private:
            /*this can live here for now. needs to be refactored somewhere else */
            void createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        private:
            std::vector<Vertex> m_vertices;
            std::vector<uint32_t> m_indices;
            Device& m_device;
            CommandBuffer& m_commandBuffer;
    };
}
