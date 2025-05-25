#include "kApplication.hpp"
#include "kCamera.hpp"
#include "frame.hpp"


namespace karhu
{
    Application::Application()
    {
        //Global UBO
        m_GlobalDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
        m_GlobalPool = m_GlobalDescriptorBuilder.createDescriptorPool(2);

        m_ObjDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_ObjDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_ObjPool = m_ObjDescriptorBuilder.createDescriptorPool(1000);

        m_UnrealObjDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_UnrealObjDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_UnrealObjPool = m_UnrealObjDescriptorBuilder.createDescriptorPool(1000);

        m_CubeMapDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        m_CubeMapDescriptorBuilder.addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_CubeMapPool = m_UnrealObjDescriptorBuilder.createDescriptorPool(1000);
    }

    Application::~Application()
    {
        vkDestroyDescriptorPool(m_Renderer.getDevice().m_Device, m_GlobalPool, nullptr);
        vkDestroyDescriptorPool(m_Renderer.getDevice().m_Device, m_ObjPool, nullptr);
        vkDestroyDescriptorSetLayout(m_Renderer.getDevice().m_Device, m_GlobalLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_Renderer.getDevice().m_Device, m_ObjLayout, nullptr);
    }

    void Application::run()
    {
        auto model = std::make_shared<kModel>(m_Renderer.getDevice(),m_Renderer.getSwapChain(), "../models/DamagedHelmet.gltf", m_Renderer.getCommandPool());
        auto model2 = std::make_shared<kModel>(m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../models/DamagedHelmet.gltf", m_Renderer.getCommandPool());
        auto cubeMap = std::make_shared<kModel>(m_Renderer.getDevice(), m_Renderer.getSwapChain(), m_CubeMapVerts, m_CubeMapIndices, m_Renderer.getCommandPool(), true);

        auto entity = kEntity::createEntity();
        entity.setModel(model);
        entity.setPosition({ 0.0f,0.0f,-50.0f });
        entity.setScale({ 1.0f,1.0f,1.0f });
        entity.setRotation({ 90.0f,0.0f,0.0f });

        auto entity2 = kEntity::createEntity();
        entity2.setModel(model);
        entity2.setPosition({ 5.0f,0.0f,-5.0f });
        entity2.setScale({ 1.0f,1.0f,1.0f });
        entity2.setRotation({ 90.0f,0.0f,0.0f });


        m_Entities.push_back(std::move(entity));
        m_Entities.push_back(std::move(entity2));

        auto unrealEntity = kEntity::createEntity();
        unrealEntity.setModel(model);
        unrealEntity.setPosition({ -5.0f,0.0f,-5.0f });
        unrealEntity.setScale({ 1.0f,1.0f,1.0f });
        unrealEntity.setRotation({ 90.0f,0.0f,0.0f });

        auto unrealEntity2 = kEntity::createEntity();
        unrealEntity2.setModel(model);
        unrealEntity2.setPosition({ -10.0f,0.0f,-5.0f });
        unrealEntity2.setScale({ 1.0f,1.0f,1.0f });
        unrealEntity2.setRotation({ 90.0f,0.0f,0.0f });

        m_UnrealEntities.push_back(std::move(unrealEntity));
        m_UnrealEntities.push_back(std::move(unrealEntity2));

        auto cubeMapEnt = kEntity::createEntity();
        cubeMapEnt.setModel(cubeMap);
        cubeMapEnt.setPosition({ 0.0f, 0.0f, -5.0f });
        cubeMapEnt.setScale({ 10.0f, 10.0f, 10.0f });

        m_CubemapEntities.push_back(std::move(cubeMapEnt));


        m_GlobalDescriptorBuilder.bind(m_GlobalBindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_GlobalLayout = m_GlobalDescriptorBuilder.createDescriptorSetLayout(m_GlobalBindings);

        std::vector<std::unique_ptr<kBuffer>> uboBuffers(2);
        for (size_t i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<kBuffer>(m_Renderer.getDevice());
            m_Renderer.createUniformBuffers(*uboBuffers[i]);
        }
        
        m_GlobalDescriptorBuilder.allocateDescriptor(m_GlobalSet, m_GlobalLayout, m_GlobalPool);
        m_GlobalDescriptorBuilder.writeBuffer(m_GlobalSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboBuffers[0]->getBufferInfo(sizeof(UniformBufferObject)), 0);
        m_GlobalDescriptorBuilder.fillWritesMap(0);

        /*Disney*/
        //obj descriptors
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ObjDescriptorBuilder.bind(m_ObjBindings, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ObjLayout = m_ObjDescriptorBuilder.createDescriptorSetLayout(m_ObjBindings);

        std::vector<std::vector<VkDescriptorImageInfo>> infos;
        infos.resize(m_Entities.size());
        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            for (size_t j = 0; j < m_Entities[i].getModel()->m_Textures.size(); j++)
            {
                infos[i].push_back(m_Entities[i].getModel()->m_Textures[j].getImageInfo());
            }
            m_Entities[i].m_Buffer = std::make_unique<kBuffer>(m_Renderer.getDevice());
            m_Entities[i].m_Buffer->createBuffer(sizeof(ObjBuffer));
        }

        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            auto id = m_Entities[i].getId();
            m_ObjDescriptorBuilder.allocateDescriptor(m_Entities[i].m_DescriptorSet, m_ObjLayout, m_ObjPool);
            m_ObjDescriptorBuilder.writeBuffer(m_Entities[i].m_DescriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_Entities[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);
         
            m_ObjDescriptorBuilder.writeImg(m_Entities[i].m_DescriptorSet, 1, infos[i][0], id);
            m_ObjDescriptorBuilder.writeImg(m_Entities[i].m_DescriptorSet, 2, infos[i][1], id);
            m_ObjDescriptorBuilder.writeImg(m_Entities[i].m_DescriptorSet, 3, infos[i][2], id);
            m_ObjDescriptorBuilder.writeImg(m_Entities[i].m_DescriptorSet, 4, infos[i][3], id);
            m_ObjDescriptorBuilder.writeImg(m_Entities[i].m_DescriptorSet, 5, infos[i][4], id);
            m_ObjDescriptorBuilder.fillWritesMap(m_Entities[i].getId());
            
        }

        /*Unreal*/
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UnrealObjDescriptorBuilder.bind(m_UnrealObjBindings, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_UnrealObjLayout = m_UnrealObjDescriptorBuilder.createDescriptorSetLayout(m_UnrealObjBindings);

        std::vector<std::vector<VkDescriptorImageInfo>> unrealinfos;
        unrealinfos.resize(m_UnrealEntities.size());
        for (size_t i = 0; i < m_UnrealEntities.size(); i++)
        {
            for (size_t j = 0; j < m_UnrealEntities[i].getModel()->m_Textures.size(); j++)
            {
                unrealinfos[i].push_back(m_UnrealEntities[i].getModel()->m_Textures[j].getImageInfo());
            }
            m_UnrealEntities[i].m_Buffer = std::make_unique<kBuffer>(m_Renderer.getDevice());
            m_UnrealEntities[i].m_Buffer->createBuffer(sizeof(ObjBuffer));
        }

        for (size_t i = 0; i < m_UnrealEntities.size(); i++)
        {
            auto id = m_UnrealEntities[i].getId();
            m_UnrealObjDescriptorBuilder.allocateDescriptor(m_UnrealEntities[i].m_DescriptorSet, m_UnrealObjLayout, m_UnrealObjPool);
            m_UnrealObjDescriptorBuilder.writeBuffer(m_UnrealEntities[i].m_DescriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_UnrealEntities[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);

            m_UnrealObjDescriptorBuilder.writeImg(m_UnrealEntities[i].m_DescriptorSet, 1, unrealinfos[i][0], id);
            m_UnrealObjDescriptorBuilder.writeImg(m_UnrealEntities[i].m_DescriptorSet, 2, unrealinfos[i][1], id);
            m_UnrealObjDescriptorBuilder.writeImg(m_UnrealEntities[i].m_DescriptorSet, 3, unrealinfos[i][2], id);
            m_UnrealObjDescriptorBuilder.writeImg(m_UnrealEntities[i].m_DescriptorSet, 4, unrealinfos[i][3], id);
            m_UnrealObjDescriptorBuilder.writeImg(m_UnrealEntities[i].m_DescriptorSet, 5, unrealinfos[i][4], id);
            m_UnrealObjDescriptorBuilder.fillWritesMap(m_UnrealEntities[i].getId());

        }

        /* CubeMap */
        m_CubeMapDescriptorBuilder.bind(m_CubeMapBindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_CubeMapDescriptorBuilder.bind(m_CubeMapBindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_CubeMapLayout = m_CubeMapDescriptorBuilder.createDescriptorSetLayout(m_CubeMapBindings);

        std::vector<std::vector<VkDescriptorImageInfo>> cubemapinfos;
        cubemapinfos.resize(1);
        for (size_t i = 0; i < m_CubemapEntities.size(); i++)
        {
            for (size_t j = 0; j < m_CubemapEntities[i].getModel()->m_Textures.size(); j++)
            {
                cubemapinfos[i].push_back(m_CubemapEntities[i].getModel()->m_Textures[j].getImageInfo());
            }
            //cubemapinfos[i].push_back(m_CubemapEntities[i].getModel()->m_Textures[i].getImageInfo());
            m_CubemapEntities[i].m_Buffer = std::make_unique<kBuffer>(m_Renderer.getDevice());
            m_CubemapEntities[i].m_Buffer->createBuffer(sizeof(ObjBuffer));
        }

        for (size_t i = 0; i < m_CubemapEntities.size(); i++)
        {
            auto id = m_CubemapEntities[i].getId();
            m_CubeMapDescriptorBuilder.allocateDescriptor(m_CubemapEntities[i].m_DescriptorSet, m_CubeMapLayout, m_CubeMapPool);
            m_CubeMapDescriptorBuilder.writeBuffer(m_CubemapEntities[i].m_DescriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_CubemapEntities[i].m_Buffer->getBufferInfo(sizeof(ObjBuffer)), id);

            m_CubeMapDescriptorBuilder.writeImg(m_CubemapEntities[i].m_DescriptorSet, 1, cubemapinfos[i][0], id);
            m_CubeMapDescriptorBuilder.fillWritesMap(m_CubemapEntities[i].getId());
        }

        m_GlobalDescriptorBuilder.createDescriptorSets(m_GlobalLayout, m_GlobalPool);
        m_ObjDescriptorBuilder.createDescriptorSets(m_Entities, m_ObjLayout, m_ObjPool);
        m_UnrealObjDescriptorBuilder.createDescriptorSets(m_UnrealEntities, m_UnrealObjLayout, m_UnrealObjPool);
        m_CubeMapDescriptorBuilder.createDescriptorSets(m_CubemapEntities, m_CubeMapLayout, m_CubeMapPool);

        std::vector<VkDescriptorSetLayout> layouts{ m_GlobalLayout, m_ObjLayout };
        std::vector<VkDescriptorSetLayout> unreallayouts{ m_GlobalLayout, m_UnrealObjLayout };
        std::vector<VkDescriptorSetLayout> cubemaplayouts{ m_GlobalLayout, m_CubeMapLayout };

        m_EntityPipeline.createGraphicsPipeline(layouts);
        m_UnrealEntityPipeline.createGraphicsPipeline(unreallayouts);
        m_CubeMapPipeline.createGraphicsPipeline(cubemaplayouts);

        update(m_DeltaTime, uboBuffers);
    }

    void Application::update(float deltaTime, std::vector<std::unique_ptr<kBuffer>>& buffers)
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
            keyboard.update(m_Renderer.getWindow(), dt, m_Camera, m_Renderer.getSwapChain().m_SwapChainExtent.width, m_Renderer.getSwapChain().m_SwapChainExtent.height);
            m_Camera.setView(cameraEntity.getPosition(), cameraEntity.getRotation(), glm::vec3(0.0f, 1.0f, 0.0f));
            m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);

            uint32_t imageIndex = 0;
            auto commandBuffer = m_Renderer.beginFrame(m_CurrentFrame, imageIndex);

            Frame frameInfo
            {
                m_CurrentFrame,
                commandBuffer,
                m_GlobalSet,
                m_Entities,
                m_UnrealEntities,
                m_CubemapEntities
            };
            m_CubeMapPipeline.renderCube(m_Camera.m_CameraVars.m_Position, frameInfo);
            m_EntityPipeline.renderEntities(m_Camera.m_CameraVars.m_Position, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), frameInfo);
            m_UnrealEntityPipeline.renderEntities(m_Camera.m_CameraVars.m_Position, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), frameInfo);

            m_Renderer.renderImguiLayer(commandBuffer, frameInfo, dt);
            m_Renderer.updateUBOs(buffers, m_Camera);
            for (auto& entity : m_Entities)
            {
                entity.updateBuffer();
            }
            
            m_Renderer.endFrame(m_CurrentFrame, imageIndex, commandBuffer);
        }
        vkDeviceWaitIdle(m_Renderer.getDevice().m_Device);
    }
} // namespace karhu
