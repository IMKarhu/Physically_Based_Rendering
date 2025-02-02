#include "kApplication.hpp"
#include "kCamera.hpp"


namespace karhu
{
    Application::Application()
    {

    }

    Application::~Application()
    {
        for (auto& entity : m_Entities)
        {
            entity.m_UniformBuffer.destroy();
        }
    }

    void Application::run()
    {
        //m_Model = std::make_unique<kModel>(m_Renderer.getDevice(), m_Vertices, m_Indices, m_Renderer.getCommandPool());
        auto model = std::make_shared<kModel>(m_Renderer.getDevice(), m_Vertices, m_Indices, m_Renderer.getCommandPool());

        auto entity = kEntity::createEntity();
        entity.setModel(model);
        //entity.setRotation({ -90.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity));

        auto entity2 = kEntity::createEntity();
        entity2.setModel(model);
        entity2.setPosition({ 1.0f,0.0f,0.0f });
        entity2.setRotation({ 0.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity2));

        m_Renderer.createUniformBuffers(m_Entities);
        m_Renderer.getDescriptor().createDescriptorSets(m_Entities);

        update(m_DeltaTime);
    }

    void Application::update(float deltaTime)
    {
        kCamera m_Camera{};
        while (!m_Renderer.getWindowShouldclose())
        {
            m_Renderer.windowPollEvents();
            m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);
            m_Camera.setView(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            
            uint32_t imageIndex = 0;
            m_Renderer.beginFrame(m_CurrentFrame, imageIndex);
            renderEntities(m_Camera, m_CurrentFrame, imageIndex);
            m_Renderer.updateUBOs(m_Entities, m_Camera);
            m_Renderer.endFrame(m_CurrentFrame, imageIndex);
            //drawFrame();
        }
        vkDeviceWaitIdle(m_Renderer.getDevice().m_Device);
    }
    void Application::renderEntities(kCamera& camera, uint32_t currentFrameIndex, uint32_t index)
    {
        m_Renderer.beginRecordCommandBuffer(currentFrameIndex, index);
        for (auto& entity : m_Entities)
        {
            m_Renderer.recordCommandBuffer(entity, m_Indices, m_CurrentFrame, index);
        }
        m_Renderer.endRecordCommandBuffer(currentFrameIndex);
    }
} // namespace karhu
