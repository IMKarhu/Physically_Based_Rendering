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
        
        

        auto model = std::make_shared<kModel>(m_Renderer.getDevice(), , m_Renderer.getCommandPool());

        auto entity = kEntity::createEntity();
        entity.setModel(model);
        //entity.setRotation({ -90.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity));

       /* auto entity2 = kEntity::createEntity();
        entity2.setModel(model);
        entity2.setPosition({ 1.0f,1.0f,.5f });
        entity2.setRotation({ 0.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity2));*/

        m_Renderer.createUniformBuffers(m_Entities);
        m_Renderer.getDescriptor().createDescriptorSets(m_Entities);

        update(m_DeltaTime);
    }

    void Application::update(float deltaTime)
    {
        auto cameraEntity = kEntity::createEntity();
        cameraEntity.setPosition({ 0.0f, 2.0f, 2.5f });
        kCamera m_Camera{};
        keyboardMovement keyboard{};
        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!m_Renderer.getWindowShouldclose())
        {
            m_Renderer.windowPollEvents();
            auto startTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
            currentTime = startTime;

            keyboard.update(m_Renderer.getWindow(), dt, cameraEntity, m_Renderer.getWindowWidth(), m_Renderer.getWindowHeight());
            m_Camera.setView(cameraEntity.getPosition(), cameraEntity.getRotation(), glm::vec3(0.0f, 1.0f, 0.0f));
            //m_Camera.setyxzView(cameraEntity.getPosition(), cameraEntity.getRotation());
            m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);
            
            uint32_t imageIndex = 0;
            m_Renderer.beginFrame(m_CurrentFrame, imageIndex);
            renderEntities(m_Camera, m_CurrentFrame, imageIndex);
            m_Renderer.updateUBOs(m_Entities, m_Camera);
            m_Renderer.endFrame(m_CurrentFrame, imageIndex);
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
