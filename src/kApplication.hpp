#pragma once
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kDescriptors.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

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

    class Application
    {
    public:
        Application();
        ~Application();
        void run();
        void createGraphicsPipeline();
        void createRenderPass();
        void createFrameBuffers();
        void createCommandPool();
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index);
        void createSyncObjects();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void updateUBOs(uint32_t currentImage);
        void update(float deltaTime);
        void drawFrame();
    private:
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        void createSurface();
        VkSwapchainCreateInfoKHR fillSwapchainCI();
        static std::vector<char> readFile(const std::string& fileName);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void cleanUpSwapChain();
        void reCreateSwapChain();
        void loadGltfFile(std::string fileName);
    private:
        std::unique_ptr<kWindow> m_Window;
        std::shared_ptr<Vulkan_Device> m_VkDevice;
        std::shared_ptr<Vulkan_SwapChain> m_VkSwapChain;
        std::shared_ptr<kDescriptors> m_Descriptor; /*= std::make_shared<kDescriptors>();*/


        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkSurfaceKHR m_Surface;
        VkRenderPass m_RenderPass;
        VkDescriptorSetLayout m_DescriptorLayout;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
        std::vector<VkDynamicState> m_DynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        std::vector<VkFramebuffer> m_FrameBuffers;
        VkCommandPool m_CommandPool;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        struct m_Semaphores
        {
            std::vector<VkSemaphore> availableSemaphores;
            std::vector<VkSemaphore> finishedSemaphores;
        }m_Semaphores;
        std::vector<VkFence> m_InFlightFences;
        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;
        const int m_MaxFramesInFlight = 2;

        const std::vector<Vertex> m_Vertices = {
            {{-0.5f,-0.5f},{1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f},{0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f},{0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f},{1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint16_t> m_Indices = {
            0,1,2,2,3,0
        };
        VkBuffer m_VertexBuffer;
        VkDeviceMemory m_VertexBufferMemory;

        VkBuffer m_IndexBuffer;
        VkDeviceMemory m_IndexBufferMemory;

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_UniformBuffersMapped;

        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };
} // namespace karhu
