#pragma once
#include "kRenderer.hpp"
#include "keyboardMovement.hpp"
#include "kEntity.hpp"
#include "pipelines/kBasicRenderSystem.hpp"
#include "pipelines/kUnrealRenderSystem.hpp"
#include "pipelines/kCubemap.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{


    class Application
    {
    public:
        Application();
        ~Application();

        
        void run();
        void update(float deltaTime, std::vector<std::unique_ptr<kBuffer>>& buffers, std::vector<std::unique_ptr<kBuffer>>& cubeMapBuffers);
    private:
        kRenderer m_Renderer{};
        kDescriptors m_GlobalDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_ObjDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_UnrealObjDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_CubeMapDescriptorBuilder{ m_Renderer.getDevice() };
        kDescriptors m_GlobalCubeDescriptorBuilder{ m_Renderer.getDevice() };
        kBasicRenderSystem m_EntityPipeline{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        kUnrealRenderSystem m_UnrealEntityPipeline{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        kCubeMap m_CubeMapPipeline{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        std::vector<kEntity> m_Entities;
        std::vector<kEntity> m_UnrealEntities;
        std::vector<kEntity> m_CubemapEntities;

        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkDescriptorSetLayoutBinding> m_GlobalBindings;
        VkDescriptorSetLayout m_GlobalLayout;
        VkDescriptorPool m_GlobalPool;
        VkDescriptorSet m_GlobalSet;

        VkDescriptorPool m_ObjPool;
        VkDescriptorSetLayout m_ObjLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_ObjBindings;

        VkDescriptorPool m_UnrealObjPool;
        VkDescriptorSetLayout m_UnrealObjLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_UnrealObjBindings;

        VkDescriptorPool m_CubeMapPool;
        VkDescriptorSetLayout m_CubeMapLayout;
        std::vector<VkDescriptorSetLayoutBinding> m_CubeMapBindings;

        std::vector<VkDescriptorSetLayoutBinding> m_GlobalCubeBindings;
        VkDescriptorSetLayout m_GlobalCubeLayout;
        VkDescriptorPool m_GlobalCubePool;
        VkDescriptorSet m_GlobalCubeSet;

        const std::vector<Vertex> m_CubeMapVerts = {
            { { -1.0f, -1.0f, -1.0f } }, // Vertex 0
            { { -1.0f, -1.0f,  1.0f } }, // Vertex 1
            { { 1.0f, -1.0f,  1.0f } }, // Vertex 2
            { { 1.0f, -1.0f, -1.0f } }, // Vertex 3
            { { -1.0f, 1.0f, -1.0f } }, // Vertex 4
            { { -1.0f,  1.0f, 1.0f } }, // Vertex 5
            { { 1.0f,  1.0f,  1.0f } }, // Vertex 6
            { { 1.0f,  1.0f, -1.0f } }  // Vertex 7
        };

        const std::vector<uint32_t> m_CubeMapIndices = {
             0, 1, 2, 2, 3, 0, // Bottom face
             4, 5, 6, 6, 7, 4, // Top face
             0, 1, 5, 5, 4, 0, // Front face
             1, 2, 6, 6, 5, 1, // Right face
             2, 3, 7, 7, 6, 2, // Back face
             3, 0, 4, 4, 7, 3  // Left face
        };
    };
} // namespace karhu
