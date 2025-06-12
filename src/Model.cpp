#include "Model.hpp"
#include "CommandBuffer.hpp"
#include <assimp/Importer.hpp>

#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

namespace karhu
{
    Model::Model(Device& device, CommandBuffer& commandBuffer, std::string filepath)
        : m_device(device)
        , m_commandBuffer(commandBuffer)
    {
        loadModel(filepath);
        std::cout << "size of vertices: " << m_vertices.size() << std::endl;
        std::cout << "size of indices: " << m_indices.size() << std::endl;
        createVertexBuffer();
        createIndexBuffer();
    }
    
    Model::Model(Device& device, CommandBuffer& commandBuffer, std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool hdr)
        : m_device(device)
        , m_commandBuffer(commandBuffer)
    {
        m_vertices = vertices;
        m_indices = indices;
        createVertexBuffer(vertices);
        createIndexBuffer(indices);
        // if (hdr)
        // {
        //     Texture tex{ m_device, "monkstown_castle_4k.hdr", VK_FORMAT_R32G32B32_SFLOAT, true};
        //     m_Textures.push_back(tex);
        // }
    }
    
    Model::~Model()
    {
        // vkDestroyBuffer(m_device.lDevice(), m_IndexBuffer.m_IndexBuffer, nullptr);
        // vkFreeMemory(m_device.lDevice(), m_IndexBuffer.m_IndexBufferMemory, nullptr);
        // vkDestroyBuffer(m_device.lDevice(), m_VertexBuffer.m_VertexBuffer, nullptr);
        // vkFreeMemory(m_device.lDevice(), m_VertexBuffer.m_VertexBufferMemory, nullptr);
        // for (auto& tex : m_Textures)
        // {
        //     vkDestroySampler(m_device.lDevice(), tex.m_TextureVars.m_Sampler, nullptr);
        //     vkDestroyImageView(m_device.lDevice(), tex.m_TextureVars.m_TextureView, nullptr);
        //     vkDestroyImage(m_device.lDevice(), tex.m_TextureVars.m_texture, nullptr);
        //     vkFreeMemory(m_device.lDevice(), tex.m_TextureVars.m_Memory, nullptr);
        // }
        
        
        printf("model Destroyed\n");
    }
    
    void Model::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer vBuffers[] = { m_vertexBuffer.m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);
        
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
    
    void Model::draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
    }
    
    void Model::loadModel(std::string filepath)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, aiProcess_CalcTangentSpace |
                aiProcess_Triangulate |
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals);
        
        if (scene == nullptr)
        {
            std::cout << "Unable to read file! error message: \n" << std::endl;
            throw std::runtime_error(importer.GetErrorString());
        }
        processScene(scene);
    }
    
    void Model::processScene(const aiScene* scene)
    {
        aiNode* node = scene->mRootNode;
        auto numMeshes = node->mNumMeshes;
        auto numMaterials = scene->mNumMaterials;
        printf("number of meshes: %d\n", numMeshes);
        printf("number of materials: %d\n", numMaterials);
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processNode(mesh, scene);
        }
    }
    
    void Model::processNode(aiMesh* mesh, const aiScene *scene)
    {
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vert;
            
            vert.pos.x = mesh->mVertices[i].x;
            vert.pos.y = mesh->mVertices[i].y;
            vert.pos.z = mesh->mVertices[i].z;
            
            vert.normal.x = mesh->mNormals[i].x;
            vert.normal.y = mesh->mNormals[i].y;
            vert.normal.z = mesh->mNormals[i].z;
            
            vert.texcoords.x = mesh->mTextureCoords[0][i].x;
            vert.texcoords.y = mesh->mTextureCoords[0][i].y;
            
            vert.color = { 1.0f, 1.0f, 0.0f };
            
            m_vertices.push_back(vert);
        }
        
        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++)
            {
                m_indices.push_back(face.mIndices[j]);
            }
        }
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            const aiMaterial* material = scene->mMaterials[i];
            
            aiString Matname = material->GetName();
            // printf("name of material: %s\n", Matname.C_Str());
            
            auto numProps = material->mNumProperties;
            // printf("number of properties: %d\n", numProps);
            for (auto j = 0; j < material->mNumProperties; j++)
            {
                aiMaterialProperty* prop = material->mProperties[j];
                aiString key = prop->mKey;
                auto sema = prop->mSemantic;
                // printf("key %s\n", key.C_Str());
                // printf("semantic %d\n", sema);
            }
        }
        if (mesh->mMaterialIndex >= 0)
        {
            aiString baseColor, metallicRoughness, normalmap, ao, emissive;
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiTextureType type = aiTextureType_NORMALS;
            aiTextureType type2 = aiTextureType_AMBIENT_OCCLUSION;
            auto i = material->GetTextureCount(type);
            auto j = material->GetTextureCount(type2);
            // printf("count of normal textures: %f", i);
            // printf("count of ao textures: %f", j);
            material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &baseColor);
            material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughness);
            material->GetTexture(aiTextureType_NORMALS, 0, &normalmap);
            material->GetTexture(aiTextureType_LIGHTMAP, 0, &ao);
            material->GetTexture(aiTextureType_EMISSIVE, 0, &emissive);
            //material->GetTexture(AI_MATKEY_COLOR_AMBIENT, &ao);
            float normalScale = 1.0f;
            //material->Get(AI_MATKEY_GLTF_NORMALS_SCALE)
            //material->GetTexture(AI_MATKEY_TEXTURE_NORMALS, &normal)
            // printf("helo\n");
            printf("BaseColor: %s \n", baseColor.C_Str());
            printf("metallicRoughness: %s \n", metallicRoughness.C_Str());
            printf("normal: %s \n", normalmap.C_Str());
            printf("ambient occlusion: %s \n", ao.C_Str());
            /*NTexture base{ m_device, m_commandBuffer, baseColor.C_Str(), VK_FORMAT_R8G8B8A8_SRGB};*/
            /*NTexture normal{ m_device, m_commandBuffer, normalmap.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };*/
            /*NTexture mr{ m_device, m_commandBuffer, metallicRoughness.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };*/
            /*NTexture amoc{ m_device, m_commandBuffer, ao.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };*/
            /*NTexture em{ m_device, m_commandBuffer, emissive.C_Str(), VK_FORMAT_R8G8B8A8_UNORM };*/

            m_Textures.emplace_back(m_device, m_commandBuffer, baseColor.C_Str(), VK_FORMAT_R8G8B8A8_SRGB);
            m_Textures.emplace_back(m_device, m_commandBuffer, normalmap.C_Str(), VK_FORMAT_R8G8B8A8_UNORM);
            m_Textures.emplace_back(m_device, m_commandBuffer, metallicRoughness.C_Str(), VK_FORMAT_R8G8B8A8_UNORM);
            m_Textures.emplace_back(m_device, m_commandBuffer, ao.C_Str(), VK_FORMAT_R8G8B8A8_UNORM);
            m_Textures.emplace_back(m_device, m_commandBuffer, emissive.C_Str(), VK_FORMAT_R8G8B8A8_UNORM);
        }
    }
    
    void Model::createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);
        
        void* data;
        VK_CHECK(vkMapMemory(m_device.lDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_device.lDevice(), stagingBufferMemory);
        
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vertexBuffer.m_vertexBuffer, m_vertexBuffer.m_vertexBufferMemory);
        
        copyBuffers(stagingBuffer, m_vertexBuffer.m_vertexBuffer, bufferSize);
        
        vkDestroyBuffer(m_device.lDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.lDevice(), stagingBufferMemory, nullptr);
    }
    
    void Model::createVertexBuffer(std::vector<Vertex>& vertices)
    {
        VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);
        
        void* data;
        VK_CHECK(vkMapMemory(m_device.lDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_device.lDevice(), stagingBufferMemory);
        
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vertexBuffer.m_vertexBuffer, m_vertexBuffer.m_vertexBufferMemory);
        
        copyBuffers(stagingBuffer, m_vertexBuffer.m_vertexBuffer, bufferSize);
        
        vkDestroyBuffer(m_device.lDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.lDevice(), stagingBufferMemory, nullptr);
    }
    
    void Model::createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);
        
        void* data;
        VK_CHECK(vkMapMemory(m_device.lDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_device.lDevice(), stagingBufferMemory);
        
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_indexBuffer.m_indexBuffer, m_indexBuffer.m_indexBufferMemory);
        
        copyBuffers(stagingBuffer, m_indexBuffer.m_indexBuffer, bufferSize);
        
        vkDestroyBuffer(m_device.lDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.lDevice(), stagingBufferMemory, nullptr);
    }
    void Model::createIndexBuffer(std::vector<uint32_t>& indices)
    {
        VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);
        
        void* data;
        VK_CHECK(vkMapMemory(m_device.lDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_device.lDevice(), stagingBufferMemory);
        
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_indexBuffer.m_indexBuffer, m_indexBuffer.m_indexBufferMemory);
        
        copyBuffers(stagingBuffer, m_indexBuffer.m_indexBuffer, bufferSize);
        
        vkDestroyBuffer(m_device.lDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.lDevice(), stagingBufferMemory, nullptr);
    }

    void Model::createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createinfo.size = size; //byte size of one vertices multiplied by size of vector
        createinfo.usage = usage;
        createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VK_CHECK(vkCreateBuffer(m_device.lDevice(), &createinfo, nullptr, &buffer));
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device.lDevice(), buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocinfo.allocationSize = memRequirements.size;
        allocinfo.memoryTypeIndex = utils::findMemoryType(m_device.pDevice(), memRequirements.memoryTypeBits, properties);
        
        VK_CHECK(vkAllocateMemory(m_device.lDevice(), &allocinfo, nullptr, &bufferMemory));
        VK_CHECK(vkBindBufferMemory(m_device.lDevice(), buffer, bufferMemory, 0));
    }

    void Model::copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandBuffer.getCommandPool();
        allocInfo.commandBufferCount = 1;
        
        VkCommandBuffer commandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_device.lDevice(), &allocInfo, &commandBuffer));
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        
        vkEndCommandBuffer(commandBuffer);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        
        VK_CHECK(vkQueueSubmit(m_device.pQueue(), 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_device.pQueue()));
        vkFreeCommandBuffers(m_device.lDevice(), m_commandBuffer.getCommandPool(), 1, &commandBuffer);
    }
}
