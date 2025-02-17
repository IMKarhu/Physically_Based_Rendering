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
        
        

        auto model = std::make_shared<kModel>(m_Renderer.getDevice(), "../models/DamagedHelmet.gltf", m_Renderer.getCommandPool());

        auto entity = kEntity::createEntity();
        entity.setModel(model);
        entity.setPosition({ 0.0f,0.0f,-5.0f });
        entity.setScale({ 1.0f,1.0f,1.0f });
        entity.setRotation({ 90.0f,0.0f,0.0f });


        m_Entities.push_back(std::move(entity));

        auto DirectionalLight = kEntity::createEntity();
        DirectionalLight.setModel(model);
        DirectionalLight.setPosition({ 1.0f, 3.0f, 1.0f });

        m_Entities.push_back(std::move(DirectionalLight));

       /* auto entity2 = kEntity::createEntity();
        entity2.setModel(model);
        entity2.setPosition({ 1.0f,1.0f,.5f });
        entity2.setRotation({ 0.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity2));*/
        kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        albedo.createTexture("../textures/Default_Albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        normal.createTexture("../textures/Default_Normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        metallicRoughness.createTexture("../textures/Default_MetalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        ao.createTexture("../textures/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        emissive.createTexture("../textures/Default_emissive.jpg", VK_FORMAT_R8G8B8A8_SRGB);

        //VK_FORMAT_R8G8B8A8_SRGB
        /*kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        albedo.createTexture("../textures/rustediron2_basecolor.png");
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        normal.createTexture("../textures/rustediron2_normal.png");
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        metallicRoughness.createTexture("../textures/rustediron2_metallic.png");
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        ao.createTexture("../textures/rustediron2_roughness.png");
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        emissive.createTexture("../textures/Default_emissive.jpg");*/

        for (int i = 0; i < m_Entities.size(); i++)
        {
            m_Entities[i].getModel()->m_Textures.push_back(albedo);
            m_Entities[i].getModel()->m_Textures.push_back(normal);
            m_Entities[i].getModel()->m_Textures.push_back(metallicRoughness);
            m_Entities[i].getModel()->m_Textures.push_back(ao);
            m_Entities[i].getModel()->m_Textures.push_back(emissive);
        }
        
        m_Renderer.createUniformBuffers(m_Entities);
        m_Renderer.getDescriptor().createDescriptorSets(m_Entities);

        update(m_DeltaTime);
    }

    void Application::update(float deltaTime)
    {
        auto cameraEntity = kEntity::createEntity();
        cameraEntity.setPosition({ 0.0f, 0.0f, -20.0f });
        kCamera m_Camera{};
        keyboardMovement keyboard{};
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        while (!m_Renderer.getWindowShouldclose())
        {
            m_Renderer.windowPollEvents();
            auto startTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
            currentTime = startTime;

            m_Camera.update(dt);
            keyboard.update(m_Renderer.getWindow(), dt, m_Camera, m_Renderer.getWindowWidth(), m_Renderer.getWindowHeight());
            m_Camera.setView(cameraEntity.getPosition(), cameraEntity.getRotation(), glm::vec3(0.0f, 1.0f, 0.0f));
            //m_Camera.setyxzView(cameraEntity.getPosition(), cameraEntity.getRotation());
            m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);

            //m_Entities[0].setRotation({ m_Entities[0].getRotation().x, dt * glm::radians(90.0f), m_Entities[0].getRotation().z});
            
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
        //m_Renderer.startImguiLayer(index);
        for (auto& entity : m_Entities)
        {
            m_Renderer.recordCommandBuffer(entity, m_CurrentFrame, index, camera.m_CameraVars.m_Position, m_Entities[1].getPosition(), glm::vec4(1.0f,1.0f,1.0f,1.0f));
        }
        m_Renderer.endRecordCommandBuffer(currentFrameIndex);
    }
} // namespace karhu
