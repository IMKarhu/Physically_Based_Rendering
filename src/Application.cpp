#include "Application.hpp"

#include "Image.hpp"
#include "Camera.hpp"
#include "keyboardMovement.hpp"
#include "frame.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"


#include <array>
#include <chrono>

namespace karhu
{
    Application::Application()
    {
        
        printf("end of application constructor\n");
    }
    Application::~Application()
    {
        cleanUpBeforeReCreate();

        for ( size_t i = 0; i < m_commandBuffer.getMaxFramesInFlight(); i++)
        {
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_finishedSemaphores[i], nullptr);
        }
    }

    void Application::run()
    {

        printf("start of run\n");
        std::vector<VkAttachmentDescription> attachments(2);
        // 0 is color 1 is depth
        attachments[0].format = m_swapChain.getSwapChainImageFormat();
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachments[1].format = m_device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subPassDesc{};
        subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDesc.colorAttachmentCount = 1;
        subPassDesc.pColorAttachments = &colorAttachmentRef;
        subPassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depDependency{};
        depDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depDependency.dstSubpass = 0;
        depDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::vector<VkSubpassDependency> dependencies = { dependency, depDependency };

        printf("before renderpass creation");
        // RenderPass renderPass{ m_device, attachments, subPassDesc, dependency };
        m_renderPasses.emplace_back(m_device, attachments, subPassDesc, dependencies);
        printf("after renderpass creation");

        createRenderPassForCubeMap();


        m_depthImage = Image(m_device.lDevice(),
                m_device.pDevice(),
                1,
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );


        karhu::createFrameBuffer(m_device.lDevice(), m_framebuffers[Normal],
                m_swapChain.getSwapChainImageviews(),
                m_renderPasses[0].getRenderPass(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                1,
                false,
                m_depthImage.getImageView());


        createSyncObjects();
        initializeImgui();

        generatePropertiesForSphere();

        auto model = std::make_shared<Model>(m_device, m_commandBuffer, "../models/DamagedHelmet.gltf", true);
        auto cube = std::make_shared<Model>(m_device, m_commandBuffer, CUBEMAPVERTS, CUBEMAPINDICES, true);
        auto spheregltf = std::make_shared<Model>(m_device, m_commandBuffer, "../models/sphere2.gltf", false);
        auto sphereMod = std::make_shared<Model>(m_device, m_commandBuffer, m_sphere, m_sphereIndices);

        auto ent1 = Entity::createEntity();
        ent1.setModel(model);
        ent1.setPosition({0.0f, 0.0f, -5.0f});
        ent1.setScale({1.0f, 1.0f, 1.0f});
        ent1.setRotation({90.0f, 0.0f, 0.0f});

        m_entities[Disney].push_back(std::move(ent1));

        float offset = 2.0f;
        float nrSpheres = 10.0f;
        float nrRows = 10.0f;
        float nrColumns = 10.0f;
        for ( size_t row = 0; row < nrRows; ++row)
        {
            for (size_t column = 0; column < nrColumns; ++column)
            {
                auto sphere = Entity::createEntity();
                sphere.setModel(spheregltf);
                sphere.setPosition({(column - (nrColumns / 2)) * offset, (row - (nrRows / 2)) * offset, -10.0f});
                sphere.setScale({0.5f, 0.5f, 0.5f});
                sphere.setRotation({0.0f, 180.0f, 0.0f});
                m_entities[Sphere].push_back(std::move(sphere));

                auto uesphere = Entity::createEntity();
                uesphere.setModel(spheregltf);
                uesphere.setPosition({15.0f, (row - (nrRows / 2)) * offset, (column - (nrColumns / 2)) * offset});
                uesphere.setScale({0.5f, 0.5f, 0.5f});
                uesphere.setRotation({0.0f, 180.0f, 0.0f});
                m_entities[SphereUE].push_back(std::move(uesphere));
            }
        }

        auto entUnreal = Entity::createEntity();
        entUnreal.setModel(model);
        entUnreal.setPosition({10.0f, 0.0f, 5.0f});
        entUnreal.setScale({1.0f, 1.0f, 1.0f});
        entUnreal.setRotation({0.0f, 0.0f, 0.0f});

        m_entities[Unreal].push_back(std::move(entUnreal));

        /*for ( int row = 0; row < nrRows; ++row)*/
        /*{*/
        /*    for (size_t column = 0; column < nrColumns; ++column)*/
        /**/
        /*    {*/
        /*        auto sphere = Entity::createEntity();*/
        /*        sphere.setModel(spheregltf);*/
        /*        sphere.setPosition({15.0f, (row - (nrRows / 2)) * offset, (column - (nrColumns / 2)) * offset});*/
        /*        sphere.setScale({0.5f, 0.5f, 0.5f});*/
        /*        sphere.setRotation({0.0f, 180.0f, 0.0f});*/
        /*        m_entities[SphereUE].push_back(std::move(sphere));*/
        /*    }*/
            /*auto sphere = Entity::createEntity();*/
            /*sphere.setModel(sphereMod);*/
            /*sphere.setPosition({15.0f, 0.0f, (i - (nrSpheres / 2)) * offset});*/
            /*sphere.setScale({1.0f, 1.0f, 1.0f});*/
            /*sphere.setRotation({0.0f, 180.0f, 0.0f});*/
        /*}*/

        auto cubeEnt = Entity::createEntity();
        cubeEnt.setModel(cube);
        cubeEnt.setPosition({0.0f, 0.0f, 0.0f});
        cubeEnt.setRotation({0.0f, 180.0f, 0.0f});


        m_cubeMapSystem.generateBrdfLut(m_renderPasses[2].getRenderPass(),
                m_framebuffers[FramebufferType::BRDFLUT],
                m_commandBuffer,
                m_iblTextures);
        m_cubeMapSystem.generateIrradianceCube(m_renderPasses[3].getRenderPass(),
                m_framebuffers[FramebufferType::IRRADIANCE],
                m_commandBuffer,
                cubeEnt,
                m_iblTextures);
        m_cubeMapSystem.generatePreFilteredCube(m_renderPasses[4].getRenderPass(),
                m_framebuffers[FramebufferType::PREFILTER],
                m_commandBuffer,
                cubeEnt,
                m_iblTextures);


        /*Probably shouldn't be here but it'll work for now..*/
        m_builder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);
        m_builder.addPoolElement(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        m_pool = m_builder.createDescriptorPool(2);

        m_builder.bind(m_bindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        m_builder.bind(m_bindings, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_builder.bind(m_bindings, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_builder.bind(m_bindings, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_builder.bind(m_bindings, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_layout = m_builder.createDescriptorSetLayout(m_bindings);

        std::vector<std::unique_ptr<Buffer>> uboBuffers(2);
        for (size_t i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<Buffer>();
            uboBuffers[i]->m_device = m_device.lDevice();
            uboBuffers[i]->m_phyiscalDevice = m_device.pDevice();
            uboBuffers[i]->createBuffer(sizeof(UniformBufferObject),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        std::vector<std::unique_ptr<Buffer>> uboBuffersSkyBox(2);
        for (size_t i = 0; i < uboBuffersSkyBox.size(); i++)
        {
            uboBuffersSkyBox[i] = std::make_unique<Buffer>();
            uboBuffersSkyBox[i]->m_device = m_device.lDevice();
            uboBuffersSkyBox[i]->m_phyiscalDevice = m_device.pDevice();
            uboBuffersSkyBox[i]->createBuffer(sizeof(UniformBufferObject),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        std::vector<VkDescriptorImageInfo> infos;
        // infos.resize(2);
        infos.push_back(m_iblTextures.m_brdfLut.imageInfo());
        infos.push_back(m_iblTextures.m_irradianceCube.imageInfo());
        infos.push_back(m_iblTextures.m_prefilteredCube.imageInfo());


        m_builder.allocateDescriptor(m_set, m_layout, m_pool);
        m_builder.writeBuffer(m_set, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboBuffers[0]->getBufferInfo(sizeof(UniformBufferObject)), 0);
        m_builder.writeImg(m_set, 1, infos[0], 0);
        m_builder.writeImg(m_set, 2, infos[1], 0);
        m_builder.writeImg(m_set, 3, infos[2], 0);
        m_builder.writeBuffer(m_set, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboBuffersSkyBox[0]->getBufferInfo(sizeof(UniformBufferObject)), 0);

        m_builder.fillWritesMap(0);
        m_builder.createDescriptorSets(m_layout, m_pool);


        m_disneySystem.createDescriptors(m_entities[Disney], m_entities[Sphere]);
        m_disneySystem.createGraphicsPipeline(m_device.lDevice(),
                m_swapChain.getSwapChainExtent(),
                m_layout,
                m_renderPasses[0].getRenderPass());

        m_unrealSystem.createDescriptors(m_entities[Unreal], m_entities[SphereUE]);
        m_unrealSystem.createGraphicsPipeline(m_device.lDevice(),
                m_swapChain.getSwapChainExtent(),
                m_layout,
                m_renderPasses[0].getRenderPass());

        m_cubeMapSystem.createDescriptors(cubeEnt, m_iblTextures);
        m_cubeMapSystem.createGraphicsPipeline(m_device.lDevice(),
                m_swapChain.getSwapChainExtent(),
                m_layout,
                m_renderPasses[0].getRenderPass());


        update(uboBuffers, uboBuffersSkyBox, cubeEnt);
    }

    void Application::update(std::vector<std::unique_ptr<Buffer>>& gBuffers, std::vector<std::unique_ptr<Buffer>>& skyBuffers, Entity& entity)
    {
        auto cameraEntity = Entity::createEntity();
        cameraEntity.setPosition({0.0f, 0.0f, 20.0f});
        Camera camera{};
        keyboardMovement movement{};
        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!m_window->windowShouldClose())
        {
            m_window->pollEvents();
            auto startTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
            currentTime = startTime;

            camera.update(dt);
            movement.update(m_window->getWindow(),
                    dt,
                    camera,
                    m_swapChain.getSwapChainExtent().width,
                    m_swapChain.getSwapChainExtent().height);
            camera.setView(cameraEntity.getPosition(),
                    cameraEntity.getRotation(),
                    glm::vec3(0.0f, 1.0f, 0.0f));
            camera.setPerspective(glm::radians(45.0f),
                    m_swapChain.getSwapChainExtent().width / (float)m_swapChain.getSwapChainExtent().height,
                    0.1f, 100.0f);


            uint32_t imageIndex = begin(m_currentFrame);

            Frame frameInfo
            {
                m_currentFrame,
                m_commandBuffer.getCommandBuffer(m_currentFrame),
                m_set,
                cameraEntity,
                m_entities[Disney],
                m_entities[Sphere],
                m_entities[Unreal],
                m_entities[SphereUE]
            };

            m_cubeMapSystem.renderSkyBox(frameInfo, entity);

            m_disneySystem.renderEntities(frameInfo);
            m_disneySystem.renderEntitiesNotextures(frameInfo);

            m_unrealSystem.renderEntities(frameInfo);
            m_unrealSystem.renderEntitiesNotextures(frameInfo);
            renderGui(frameInfo);

            end(m_currentFrame, imageIndex);

            updateBuffers(gBuffers, skyBuffers, camera, true);
            // m_cubeMapSystem.updateCubeUbo(entity, camera);
            for (auto& entity : m_entities[Disney])
            {
                entity.updateBuffer();
            }

        }
        VK_CHECK(vkDeviceWaitIdle(m_device.lDevice()));
    }

    uint32_t Application::begin(uint32_t currentFrameIndex)
    {
        uint32_t imageIndex;
        VK_CHECK(vkWaitForFences(m_device.lDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX));

        VkResult res = vkAcquireNextImageKHR(m_device.lDevice(),
                m_swapChain.getSwapchain(),
                UINT64_MAX,
                m_semaphores.m_availableSemaphores[m_currentFrame],
                VK_NULL_HANDLE,
                &imageIndex);

        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            printf("swapchain out of date\n");
            reCreateSwapChain();
            return 0;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_device.lDevice(), 1, &m_inFlightFences[m_currentFrame]);

        m_commandBuffer.resetCommandBuffer(m_currentFrame);

        m_commandBuffer.beginCommand(m_currentFrame);

        std::vector<VkClearValue> clearValues;
        clearValues.resize(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        m_renderPasses[0].beginRenderPass(m_framebuffers[FramebufferType::Normal][imageIndex],
                m_swapChain.getSwapChainExtent(),
                clearValues,
                m_commandBuffer.getCommandBuffer(m_currentFrame),
                VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_swapChain.getSwapChainExtent().width);
        viewPort.height = static_cast<float>(m_swapChain.getSwapChainExtent().height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(m_commandBuffer.getCommandBuffer(m_currentFrame), 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.getSwapChainExtent();

        vkCmdSetScissor(m_commandBuffer.getCommandBuffer(m_currentFrame), 0, 1, &scissor);

        return imageIndex;
    }

    void Application::end(uint32_t currentFrameIndex, uint32_t imageIndex)
    {
        vkCmdEndRenderPass(m_commandBuffer.getCommandBuffer(m_currentFrame));

        m_commandBuffer.endCommandBuffer(m_currentFrame);

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_semaphores.m_availableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_commandBuffer.getCommandBuffer(m_currentFrame);
        VkSemaphore signalSemaphores[] = { m_semaphores.m_finishedSemaphores[m_currentFrame] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitinfo, m_inFlightFences[m_currentFrame]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_swapChain.getSwapchain() };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &imageIndex;
        presentinfo.pResults = nullptr;

        VkResult res = vkQueuePresentKHR(m_device.pQueue(), &presentinfo);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_window->resized())
        {
            // printf("out of date or suboptimal or window was resized\n");
            std::cout << "res:" << res << std::endl;
            m_window->setResized(false);
            reCreateSwapChain();
        }
        else if (res != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapchain image1\n");
        }
        m_currentFrame = (m_currentFrame + 1) % m_commandBuffer.getMaxFramesInFlight();
    }

    void Application::updateBuffers(std::vector<std::unique_ptr<Buffer>>& gBuffers,
            std::vector<std::unique_ptr<Buffer>>& skyBuffers,
            Camera& camera,
            bool flipY)
    {
        for( auto& buffer : gBuffers)
        {
            UniformBufferObject obj{};

            obj.view = camera.getView();
            obj.proj = camera.getProjection();
            if(flipY)
            {
                obj.proj[1][1] *= -1;
            }

            memcpy(buffer->m_bufferMapped, &obj, sizeof(obj));
        }
        for( auto& buffer : skyBuffers)
        {
            UniformBufferObject obj{};

            glm::mat4 view = glm::mat4(glm::mat3(camera.getView()));
            obj.view = view;
            obj.proj = camera.getProjection();
            memcpy(buffer->m_bufferMapped, &obj, sizeof(obj));
        }
    }

    void Application::createSyncObjects()
    {
        m_semaphores.m_availableSemaphores.resize(m_commandBuffer.getMaxFramesInFlight());
        m_semaphores.m_finishedSemaphores.resize(m_commandBuffer.getMaxFramesInFlight());
        m_inFlightFences.resize(m_commandBuffer.getMaxFramesInFlight());

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo FcreateInfo{};
        FcreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FcreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_commandBuffer.getMaxFramesInFlight(); i++)
        {
            if(vkCreateSemaphore(m_device.lDevice(), &createInfo, nullptr, &m_semaphores.m_availableSemaphores[i]) != VK_SUCCESS 
                    || vkCreateSemaphore(m_device.lDevice(), &createInfo, nullptr, &m_semaphores.m_finishedSemaphores[i]) != VK_SUCCESS
                    || vkCreateFence(m_device.lDevice(), &FcreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error(" Failed to create Semaphores or fence!\n");
            }
        }
    }

    void Application::cleanUpBeforeReCreate()
    {
        /*vkDestroyImageView(m_device.lDevice(), m_depthImage.getImageView(), nullptr);*/
        /*vkDestroyImage(m_device.lDevice(), m_depthImage.getImage(), nullptr);*/
        for (auto frameBuffer : m_framebuffers[FramebufferType::Normal])
        {
            vkDestroyFramebuffer(m_device.lDevice(), frameBuffer, nullptr);
        }
        for (auto view : m_swapChain.getSwapChainImageviews())
        {
            vkDestroyImageView(m_device.lDevice(), view, nullptr);
        }
        vkDestroySwapchainKHR(m_device.lDevice(), m_swapChain.getSwapchain(), nullptr);
    }

    void Application::reCreateSwapChain()
    {
        int width = 0;
        int height = 0;
        m_window->frameBufferSize(m_window->getWindow(), width, height);

        while (width == 0 || height == 0)
        {
            if (m_window->windowShouldClose())
            {
                return;
            }
            m_window->frameBufferSize(m_window->getWindow(), width, height);
            m_window->waitEvents();
        }

        VK_CHECK(vkDeviceWaitIdle(m_device.lDevice()));

        cleanUpBeforeReCreate();

        printf("size of framebuffers: %lld\n", m_framebuffers.size());
        printf("size of imageviews %lld\n", m_swapChain.getSwapChainImageviews().size());

        m_swapChain.createSwapChain();
        m_swapChain.createSwapChainImageViews();

        m_depthImage = Image(m_device.lDevice(),
                m_device.pDevice(),
                1,
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        karhu::createFrameBuffer(m_device.lDevice(), m_framebuffers[FramebufferType::Normal],
                m_swapChain.getSwapChainImageviews(),
                m_renderPasses[0].getRenderPass(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                1,
                false,
                m_depthImage.getImageView());
    }

    void Application::createRenderPassForCubeMap()
    {
        std::vector<VkAttachmentDescription> colorAttachment(1);
        colorAttachment[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRef;

        std::vector<VkSubpassDependency> dependency(1);
        dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency[0].dstSubpass = 0;
        dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency[0].srcAccessMask = 0;
        dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        m_renderPasses.emplace_back(m_device, colorAttachment, subpass, dependency);


        // FB, Att, RP, Pipe, etc.
        std::vector<VkAttachmentDescription> attDesc(1);
        // Color attachment
        attDesc[0].format = VK_FORMAT_R16G16_SFLOAT;
        attDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        
        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        
        // Use subpass dependencies for layout transitions
        std::vector<VkSubpassDependency> dependencies(2);
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        m_renderPasses.emplace_back(m_device, attDesc, subpassDescription, dependencies);

        //irradiance cube renderPass
        // FB, Att, RP, Pipe, etc.
        std::vector<VkAttachmentDescription> irAttDesc(1);
        // Color attachment
        irAttDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        irAttDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        irAttDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        irAttDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        irAttDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        irAttDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        irAttDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        irAttDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference irColorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        
        VkSubpassDescription irSubpassDescription = {};
        irSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        irSubpassDescription.colorAttachmentCount = 1;
        irSubpassDescription.pColorAttachments = &irColorReference;
        
        // Use subpass dependencies for layout transitions
        std::vector<VkSubpassDependency> irDependencies(2);
        irDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        irDependencies[0].dstSubpass = 0;
        irDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        irDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        irDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        irDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        irDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        irDependencies[1].srcSubpass = 0;
        irDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        irDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        irDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        irDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        irDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        irDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        m_renderPasses.emplace_back(m_device, irAttDesc, irSubpassDescription, irDependencies);

        std::vector<VkAttachmentDescription> pfAttDesc(1);
        // Color attachment
        pfAttDesc[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        pfAttDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        pfAttDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        pfAttDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        pfAttDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        pfAttDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        pfAttDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        pfAttDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference pfColorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        
        VkSubpassDescription pfSubpassDescription = {};
        pfSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        pfSubpassDescription.colorAttachmentCount = 1;
        pfSubpassDescription.pColorAttachments = &pfColorReference;
        
        // Use subpass dependencies for layout transitions
        std::vector<VkSubpassDependency> pfDependencies(2);
        pfDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        pfDependencies[0].dstSubpass = 0;
        pfDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        pfDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        pfDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        pfDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        pfDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        pfDependencies[1].srcSubpass = 0;
        pfDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        pfDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        pfDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        pfDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        pfDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        pfDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        m_renderPasses.emplace_back(m_device, pfAttDesc, pfSubpassDescription, pfDependencies);

    }

    void Application::generatePropertiesForSphere()
    {
        const unsigned int x_seg = 64;
        const unsigned int y_seg = 64;
        const double pi = 3.14159265359;
        float radius = 1.0;

        for (int i = 0; i <= x_seg; ++i)
        {
            float phi = i * pi / x_seg;
            for (int j = 0; j <= y_seg; ++j)
            {
                Vertex vert;
                float theta = j * (2.0*pi) / y_seg;
                float x = radius * std::sin(phi) * std::cos(theta);
                float y = radius * std::cos(phi);
                float z = radius * std::sin(phi) * std::sin(theta);

                vert.pos = glm::vec3(x, y, z);
                vert.normal = glm::vec3(x, y, z);
                vert.color = glm::vec3(1.0f, 0.765557f, 0.336057f);
                m_sphere.push_back(vert);
            }
        }
        int vertsPerRow = y_seg + 1;
        for(int i = 0; i < x_seg; ++i)
        {
            for(int j = 0; j < y_seg; ++j)
            {
                int row1 = i * vertsPerRow;
                int row2 = (i+1) * vertsPerRow;

                m_sphereIndices.push_back(row1 + j);
                m_sphereIndices.push_back(row2 + j);
                m_sphereIndices.push_back(row1 + j + 1);

                m_sphereIndices.push_back(row1 + j + 1);
                m_sphereIndices.push_back(row2 + j);
                m_sphereIndices.push_back(row2 + j + 1);
            }
        }
    }
    void Application::initializeImgui()
    {
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(poolSizes);
        pool_info.pPoolSizes = poolSizes;

        VK_CHECK(vkCreateDescriptorPool(m_device.lDevice(), &pool_info, nullptr, &m_guiPool));


        //initialize imgui

        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForVulkan(m_window->getWindow(), true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_window->getInstance();
        initInfo.PhysicalDevice = m_device.pDevice();
        initInfo.Device = m_device.lDevice();
        initInfo.Queue = m_device.gQueue();
        initInfo.DescriptorPool = m_guiPool;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.RenderPass = m_renderPasses[0].getRenderPass();

        ImGui_ImplVulkan_Init(&initInfo);

        VkCommandBuffer commandBuffer = m_commandBuffer.recordSingleCommand();
        ImGui_ImplVulkan_CreateFontsTexture();
        m_commandBuffer.endSingleCommand(commandBuffer);

        vkDeviceWaitIdle(m_device.lDevice());
        ImGui_ImplVulkan_DestroyFontsTexture();

    }
    void Application::renderGui(Frame& frameInfo)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        /*auto entPos = frameInfo.entities[1].getPosition();*/
        ImGui::NewFrame();

        static bool ibl = m_disneySystem.isIbl();
        static bool albedo = m_disneySystem.isAlbedo();
        static bool phong = m_disneySystem.isPhong();
        static bool blinnphong = m_disneySystem.isBlinnPhong();
       // ImGui::ShowDemoWindow();
        ImGui::Begin("Controls");
        ImGui::Checkbox("ibl", &ibl);
        ImGui::Checkbox("albedo", &albedo);
        ImGui::Checkbox("phong", &phong);
        ImGui::Checkbox("Blinn-Phong", &blinnphong);
        /*ImGui::SliderFloat("Metalness", &vars.m_Metalness, 0.0f, 1.0f);*/
        /*ImGui::SliderFloat("Roughness", &vars.m_Roughness, 0.0f, 1.0f);*/
        /*ImGui::SliderFloat3("lightPosition", glm::value_ptr(vars.m_LightPosition), -50.0f, 50.0f);*/

        /*ImGui::SliderFloat3("objPosition", glm::value_ptr(entPos), -5.0f, 200.0f);*/
        ImGui::End();

        if(ibl != m_disneySystem.isIbl())
        {
            m_disneySystem.setIblActive(ibl);
        }
        if(albedo != m_disneySystem.isAlbedo())
        {
            m_disneySystem.setAlbedoActive(albedo);
        }
        if(phong != m_disneySystem.isPhong())
        {
            m_disneySystem.setPhongActive(phong);
        }
        if(blinnphong != m_disneySystem.isBlinnPhong())
        {
            m_disneySystem.setBlinnPhongActive(blinnphong);
        }

        /*frameInfo.entities[1].setPosition(entPos);*/

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    }
} // namespace karhu
