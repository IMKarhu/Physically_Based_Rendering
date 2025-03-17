#include "kApplication.hpp"
#include "kCamera.hpp"
#include "pipelines/kBasicRenderSystem.hpp"
#include "frame.hpp"
#include "kBuffer.hpp"

#include <utility>


namespace karhu
{
    Application::Application()
    {
        m_GlobalPool = LveDescriptorPool::Builder(m_Renderer.getDevice())
            .setMaxSets(Vulkan_SwapChain::m_MaxFramesInFlight)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Vulkan_SwapChain::m_MaxFramesInFlight)
            //.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Vulkan_SwapChain::m_MaxFramesInFlight)
            .build();

        m_ObjectPools.resize(Vulkan_SwapChain::m_MaxFramesInFlight);
        auto builder = LveDescriptorPool::Builder(m_Renderer.getDevice())
            .setMaxSets(1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

        for (size_t i = 0; i < m_ObjectPools.size(); i++)
        {
            m_ObjectPools[i] = builder.build();
        }

        auto model = std::make_shared<kModel>(m_Renderer.getDevice(), "../models/DamagedHelmet.gltf", m_Renderer.getCommandPool());

        auto entity = kEntity::createEntity();
        entity.setModel(model);
        entity.setPosition({ 0.0f,0.0f,-5.0f });
        entity.setScale({ 1.0f,1.0f,1.0f });
        entity.setRotation({ 90.0f,0.0f,0.0f });


        m_Entities.emplace(entity.getID(), std::move(entity));

        kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../textures/Default_Albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB };
        //albedo.createTexture("../textures/Default_Albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../textures/Default_Normal.jpg", VK_FORMAT_R8G8B8A8_UNORM };
        //normal.createTexture("../textures/Default_Normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../textures/Default_MetalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM };
        //metallicRoughness.createTexture("../textures/Default_MetalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../textures/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM };
        //ao.createTexture("../textures/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain(), "../textures/Default_emissive.jpg", VK_FORMAT_R8G8B8A8_SRGB };
        //emissive.createTexture("../textures/Default_emissive.jpg", VK_FORMAT_R8G8B8A8_SRGB);

         for (auto& kv : m_Entities)
        {
             auto& entity = kv.second;
             entity.getModel()->m_Textures.push_back(albedo);
             entity.getModel()->m_Textures.push_back(normal);
             entity.getModel()->m_Textures.push_back(metallicRoughness);
             entity.getModel()->m_Textures.push_back(ao);
             entity.getModel()->m_Textures.push_back(emissive);

             for (size_t i = 0; i < entity.m_EntityUbo.size(); i++)
             {
                 entity.m_EntityUbo[i] = std::make_unique<kBuffer>(m_Renderer.getDevice(),
                     sizeof(EntityData),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                 entity.m_EntityUbo[i]->map(sizeof(EntityData), 0);
             }
        }
    }

    Application::~Application()
    {
        
    }

    void Application::run()
    {
        /*
       std::array<VkDescriptorPoolSize, 2> poolSize{};
       poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       poolSize[0].descriptorCount = entityCount; //count of objects?
       poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
       poolSize[1].descriptorCount = entityCount;*/
       // m_Renderer.getDescriptor().addPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
       // m_Renderer.getDescriptor().addPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
       // m_GlobalPool = m_Renderer.getDescriptor().createDescriptorPool(2); //change this to maxframesinFlight

        

        std::vector<std::unique_ptr<kBuffer>> uboBuffers(Vulkan_SwapChain::m_MaxFramesInFlight);
        for (size_t i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<kBuffer>(m_Renderer.getDevice(),
                sizeof(UniformBufferObject),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map(sizeof(UniformBufferObject), 0);
        }

        auto globalsetlayout = LveDescriptorSetLayout::Builder(m_Renderer.getDevice())
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorImageInfo> imageInfo;
        for (auto& kv : m_Entities)
        {
            auto& entity = kv.second;
            for (size_t i = 0; i < entity.getModel()->m_Textures.size(); i++)
            {
                //VkDescriptorImageInfo info = entity.getModel()->m_Textures[i].getImageinfo();
                VkDescriptorImageInfo info{};
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                info.imageView = entity.getModel()->m_Textures[i].m_TextureVars.m_TextureView;
                info.sampler = entity.getModel()->m_Textures[i].m_TextureVars.m_Sampler;
                imageInfo.push_back(info);
            }
        }
        std::vector<VkDescriptorSet> globalDescriptorSets(2);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->bufferInfo();
            LveDescriptorWriter(*globalsetlayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        
        /*.writeImage(1, &imageInfo[0])
            .writeImage(2, &imageInfo[1])
            .writeImage(3, &imageInfo[2])
            .writeImage(4, &imageInfo[3])
            .writeImage(5, &imageInfo[4])*/

         //gloabl ubo
        /*for (size_t i = 1; i < 5; i++)
        {
            m_Renderer.getDescriptor().addBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        }*/
        /*m_Renderer.getDescriptor().prepareBuffer(m_DescriptorBufferProperties);
        m_Renderer.getDescriptor().addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_GlobalSetlayout = m_Renderer.getDescriptor().createDescriptorSetLayout();
        m_Renderer.getDescriptor().freeBindings();
        m_Renderer.getDescriptor().addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_ImageSetLayout = m_Renderer.getDescriptor().createDescriptorSetLayout();

        const std::array<VkDescriptorSetLayout, 3> layouts{ m_GlobalSetlayout, m_GlobalSetlayout, m_ImageSetLayout };

        entityPipeline.createGraphicsPipeline(layouts);

        vkGetDescriptorSetLayoutSizeEXT(m_Renderer.getDevice().m_Device, m_GlobalSetlayout, &m_GlobalLayoutSize);
        vkGetDescriptorSetLayoutSizeEXT(m_Renderer.getDevice().m_Device, m_ImageSetLayout, &m_ImageLayoutSize);

        m_GlobalLayoutSize = aligned_size(m_GlobalLayoutSize, m_DescriptorBufferProperties.descriptorBufferOffsetAlignment);
        m_ImageLayoutSize = aligned_size(m_ImageLayoutSize, m_DescriptorBufferProperties.descriptorBufferOffsetAlignment);

        vkGetDescriptorSetLayoutBindingOffsetEXT(m_Renderer.getDevice().m_Device, m_GlobalSetlayout, 0u, &m_GlobalLayoutSizeOffset);
        vkGetDescriptorSetLayoutBindingOffsetEXT(m_Renderer.getDevice().m_Device, m_ImageSetLayout, 0u, &m_ImageLayoutSizeOffset);*/

        

        /*auto DirectionalLight = kEntity::createEntity();
        DirectionalLight.setModel(model);
        DirectionalLight.setPosition({ 1.0f, 3.0f, 1.0f });

        m_Entities.push_back(std::move(DirectionalLight));*/

       /* auto entity2 = kEntity::createEntity();
        entity2.setModel(model);
        entity2.setPosition({ 1.0f,1.0f,.5f });
        entity2.setRotation({ 0.0f,0.0f,0.0f });

        m_Entities.push_back(std::move(entity2));*/
        /*kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        albedo.createTexture("../textures/Default_Albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        normal.createTexture("../textures/Default_Normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        metallicRoughness.createTexture("../textures/Default_MetalRoughness.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        ao.createTexture("../textures/Default_AO.jpg", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        emissive.createTexture("../textures/Default_emissive.jpg", VK_FORMAT_R8G8B8A8_SRGB);*/

        //VK_FORMAT_R8G8B8A8_SRGB //VK_FORMAT_R8G8B8A8_UNORM
        /*kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        albedo.createTexture("../textures/rustediron2_basecolor.png", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        normal.createTexture("../textures/rustediron2_normal.png", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        metallicRoughness.createTexture("../textures/rustediron2_metallic.png", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        ao.createTexture("../textures/rustediron2_roughness.png", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        emissive.createTexture("../textures/sloppy-mortar-stone-wall_ao.png", VK_FORMAT_R8G8B8A8_SRGB);*/

       /* kTexture albedo{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        albedo.createTexture("../textures/sloppy-mortar-stone-wall_albedo.png", VK_FORMAT_R8G8B8A8_SRGB);
        kTexture normal{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        normal.createTexture("../textures/sloppy-mortar-stone-wall_normal-ogl.png", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture metallicRoughness{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        metallicRoughness.createTexture("../textures/sloppy-mortar-stone-wall_metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture ao{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        ao.createTexture("../textures/sloppy-mortar-stone-wall_roughness.png", VK_FORMAT_R8G8B8A8_UNORM);
        kTexture emissive{ m_Renderer.getDevice(), m_Renderer.getSwapChain() };
        emissive.createTexture("../textures/sloppy-mortar-stone-wall_ao.png", VK_FORMAT_R8G8B8A8_SRGB);*/

       /* for (int i = 0; i < m_Entities.size(); i++)
        {
            m_Entities[i].getModel()->m_Textures.push_back(albedo);
            m_Entities[i].getModel()->m_Textures.push_back(normal);
            m_Entities[i].getModel()->m_Textures.push_back(metallicRoughness);
            m_Entities[i].getModel()->m_Textures.push_back(ao);
            m_Entities[i].getModel()->m_Textures.push_back(emissive);
        }*/
        
        /*m_Renderer.createUniformBuffers(m_GlobalUBO);
        VkDescriptorSet globalset{};
        m_Renderer.getDescriptor().allocateSet(m_GlobalPool, globalsetlayout, globalset);
        m_Renderer.getDescriptor().write(m_GlobalUBO, globalset);

        for (auto& entity : m_Entities)
        {
            m_Renderer.getDescriptor().allocateSet(m_GlobalPool, globalsetlayout, entity.m_DescriptorSet);
            m_Renderer.getDescriptor().writeImg(entity);
        }
        m_Renderer.getDescriptor().updateDescriptorSet();*/

        /*auto size = static_cast<uint32_t>(m_Entities.size()) * m_GlobalLayoutSize;
        m_Renderer.getDevice().createBuffers(size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_GlobalBuffer.m_Buffer, m_GlobalBuffer.m_BufferMemory);

        size = static_cast<uint32_t>(m_Entities.size()) * m_ImageLayoutSize;
        m_Renderer.getDevice().createBuffers(size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_ImageBuffer.m_Buffer, m_ImageBuffer.m_BufferMemory);

        for (size_t i = 0; i < m_Entities.size(); i++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.sampler = m_Entities[0].getModel()->m_Textures[0].m_TextureVars.m_Sampler;
            imageInfo.imageView = m_Entities[0].getModel()->m_Textures[0].m_TextureVars.m_TextureView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorGetInfoEXT imageDescriptorInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
            imageDescriptorInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            imageDescriptorInfo.data.pCombinedImageSampler = &imageInfo;

            vkGetDescriptorEXT(m_Renderer.getDevice().m_Device, &imageDescriptorInfo,
                m_DescriptorBufferProperties.combinedImageSamplerDescriptorSize, i* m_ImageLayoutSize * m_ImageLayoutSizeOffset);
        }*/

        entityPipeline.createGraphicsPipeline(globalsetlayout->getDescriptorSetLayout());

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
            //m_Camera.setyxzView(cameraEntity.getPosition(), cameraEntity.getRotation());
            m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);

            uint32_t imageIndex = 0;
            auto commandBuffer = m_Renderer.beginFrame(m_CurrentFrame, imageIndex);
            Frame frameInfo{
                m_CurrentFrame,
                commandBuffer,
                globalDescriptorSets[m_CurrentFrame],
                *m_ObjectPools[m_CurrentFrame],
                m_Entities
            };

            for (auto& kv : m_Entities)
            {
                auto& entity = kv.second;
                entity.updateBuffer(m_CurrentFrame);
            }

            UniformBufferObject ubo{};
            //ubo.model = m_Entities.;
            ubo.proj = m_Camera.getProjection();
            ubo.view = m_Camera.getView();
            ubo.proj[1][1] *= -1;

            uboBuffers[m_CurrentFrame]->memcopy(&ubo, sizeof(ubo));
            entityPipeline.renderEntities(m_Camera.m_CameraVars.m_Position, glm::vec3(1.0f, 3.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), frameInfo);

            

             //m_Renderer.renderImguiLayer(m_CurrentFrame);
            m_Renderer.endFrame(m_CurrentFrame, imageIndex, commandBuffer);
        }
        vkDeviceWaitIdle(m_Renderer.getDevice().m_Device);
        

        //update(m_DeltaTime, globalDescriptorSets, uboBuffers);
    }

    void Application::update(float deltaTime, std::vector<VkDescriptorSet> sets, std::vector<std::unique_ptr<kBuffer>> uboBuffers)
    {
        //auto cameraEntity = kEntity::createEntity();
        //cameraEntity.setPosition({ 0.0f, 0.0f, -20.0f });
        //kCamera m_Camera{};
        //keyboardMovement keyboard{};
        //auto currentTime = std::chrono::high_resolution_clock::now();

        //
        //
        //while (!m_Renderer.getWindowShouldclose())
        //{
        //    m_Renderer.windowPollEvents();
        //    auto startTime = std::chrono::high_resolution_clock::now();
        //    float dt = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
        //    currentTime = startTime;

        //    m_Camera.update(dt);
        //    keyboard.update(m_Renderer.getWindow(), dt, m_Camera, m_Renderer.getSwapChain().m_SwapChainExtent.width, m_Renderer.getSwapChain().m_SwapChainExtent.height);
        //    m_Camera.setView(cameraEntity.getPosition(), cameraEntity.getRotation(), glm::vec3(0.0f, 1.0f, 0.0f));
        //    //m_Camera.setyxzView(cameraEntity.getPosition(), cameraEntity.getRotation());
        //    m_Camera.setPerspective(glm::radians(45.0f), m_Renderer.getSwapChain().m_SwapChainExtent.width / (float)m_Renderer.getSwapChain().m_SwapChainExtent.height, 0.1f, 100.0f);

        //    uint32_t imageIndex = 0;
        //    auto commandBuffer = m_Renderer.beginFrame(m_CurrentFrame, imageIndex);
        //    Frame frameInfo{
        //        m_CurrentFrame,
        //        commandBuffer,
        //        sets[m_CurrentFrame],
        //        m_Entities
        //    };

        //    UniformBufferObject ubo{};
        //   // ubo.model = m_Entities[0].getTransformMatrix();
        //    ubo.proj = m_Camera.getProjection();
        //    ubo.view = m_Camera.getView();
        //    ubo.proj[1][1] *= -1;

        //    uboBuffers[m_CurrentFrame]->memcopy(&ubo, sizeof(ubo));
        //   // m_Renderer.updateUBOs(uboBuffers[m_CurrentFrame], ubo);
        //    entityPipeline.renderEntities(m_Camera.m_CameraVars.m_Position, glm::vec3(1.0f, 3.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), frameInfo);

        //   // m_Renderer.renderImguiLayer(m_CurrentFrame, m_Entities[0]);
        //    m_Renderer.endFrame(m_CurrentFrame, imageIndex, commandBuffer);
        //}
        //vkDeviceWaitIdle(m_Renderer.getDevice().m_Device);
    }
    void Application::renderEntities(kCamera& camera, uint32_t currentFrameIndex, uint32_t index)
    {
        //m_Renderer.beginRecordCommandBuffer(currentFrameIndex, index);

        /* CREATE RENDER SYSTEMS
           RENDER ENTITIES
        */

        /*for (auto& entity : m_Entities)
        {
            m_Renderer.recordCommandBuffer(entity, m_CurrentFrame, index, camera.m_CameraVars.m_Position, glm::vec3(1.0f, 3.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }*/
        
        /*m_Renderer.endRecordCommandBuffer(currentFrameIndex);*/
    }
} // namespace karhu
