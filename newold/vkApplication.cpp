#include "vkApplication.h"
#include "vkUtils/imageViews.h"
namespace kge
{
	vkApplication::vkApplication()
	{
		std::cout << "Default Constructor called!\n" << std::endl;
	}

	vkApplication::~vkApplication()
	{
        cleanUpSwapChain();
        vkDestroyBuffer(m_Device->getDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_Device->getDevice(), m_VertexBufferMemory, nullptr);
        vkDestroyBuffer(m_Device->getDevice(), m_IndexBuffer, nullptr);
        vkFreeMemory(m_Device->getDevice(), m_IndexBufferMemory, nullptr);
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_Device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_Device->getDevice(), m_InFlightFences[i], nullptr);
        }
		vkDestroyCommandPool(m_Device->getDevice(), m_Command->getCommandPool(), nullptr);
		std::cout << "Destructor called!\n" << std::endl;
		vkDestroyPipeline(m_Device->getDevice(), m_PipeLine->getPipeLine(), nullptr);
		vkDestroyPipelineLayout(m_Device->getDevice(), m_PipeLine->getPipeLineLayout(), nullptr);
		vkDestroyRenderPass(m_Device->getDevice(), m_PipeLine->getRenderPass(), nullptr);
		vkDestroySwapchainKHR(m_Device->getDevice(), m_SwapChain->getSwapChain(), nullptr);
		delete m_SwapChain;
		vkDestroySurfaceKHR(m_Instance->getInstance(), m_Device->getSurface(), nullptr);
		delete m_Device;
		m_Instance->cleanUp(enableValidationLayers);
		delete m_Instance;
	}

	void vkApplication::run()
	{
		std::cout << "Run!\n" << std::endl;
		initVulkan();
		while (!m_Instance->shouldClose())
		{
			m_Instance->pollEvents();
			drawFrame();
		}
        vkDeviceWaitIdle(m_Device->getDevice());
	}

	void vkApplication::drawFrame()
	{
        vkWaitForFences(m_Device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device->getDevice(), m_SwapChain->getSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame],
            VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image!");
        }

        m_UniformBuffers->updateUniformBuffers(m_CurrentFrame, m_SwapChain->getSwapChainExtent());
        vkResetFences(m_Device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_Command->getCommandBuffers()[m_CurrentFrame], 0);
        m_Command->recordCommandBuffer(m_SwapChain->getSwapChainExtent(),m_PipeLine->getSCFrameBuffers(),
            m_PipeLine->getRenderPass(), m_Command->getCommandBuffers()[m_CurrentFrame], imageIndex,
            m_PipeLine->getPipeLine(), m_VertexBuffer, m_IndexBuffer,m_Descriptor->getDSsets(),
            m_CurrentFrame, static_cast<uint32_t>(m_Meshes[0]->getIndices().size()), m_PipeLine->getPipeLineLayout());

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_Command->getCommandBuffers()[m_CurrentFrame];

        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_SwapChain->getSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_Device->getPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Instance->getFrameBufferResize())
        {
            m_Instance->setFrameBufferResize(false);
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
	}

    void vkApplication::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Instance->getWindow(), &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_Instance->getWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Device->getDevice()); /* We shouldnt touch on resources that may still be in use, thus we wait. */
        cleanUpSwapChain();
        m_SwapChain->createSwapChain(m_Device->getDevice(),m_Device->getPDevice(),m_Device->getSurface(),m_Instance->getWindow());
        m_SwapChain->createSwapChainImageViews(m_Device->getDevice()); /* Image views need to be recreated because they are directly based on the swapchain images. */
        m_Model->createDepthResources(m_Device->getPDevice(), m_Device->getDevice(), m_Command->getCommandPool(),m_Device->getGraphicsQueue());
        m_PipeLine->createFrameBuffers(); /* Same as image views. */
    }

    void vkApplication::cleanUpSwapChain()
    {
        //m_Texture->cleanUpForRecreation(m_Device->getDevice());
        for (size_t i = 0; i < m_PipeLine->getSCFrameBuffers().size(); i++) {
            vkDestroyFramebuffer(m_Device->getDevice(), m_PipeLine->getSCFrameBuffers()[i], nullptr);
        }
        for (size_t i = 0; i < m_SwapChain->getSCImageViews().size(); i++) {
            vkDestroyImageView(m_Device->getDevice(), m_SwapChain->getSCImageViews()[i], nullptr);
        }
        vkDestroySwapchainKHR(m_Device->getDevice(), m_SwapChain->getSwapChain(), nullptr);
    }

    void vkApplication::createSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(m_MaxFramesInFlight);
        m_RenderFinishedSemaphores.resize(m_MaxFramesInFlight);
        m_InFlightFences.resize(m_MaxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            if (vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_Device->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_Device->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    Mesh* vkApplication::processMesh(aiMesh* mesh)
    {
        std::vector<Vertex> vertices;//TODO 1: add data containers for vertices and indices
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex; //temporable container for the data of each loop
            glm::vec3 vector;//TODO 2: load data from the Assimp mesh to our containers

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;

            /*vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normals = vector;*/

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoord = vec;
                /*vertex.texCoord = {
                    mesh->mTextureCoords[2 * mesh->mMaterialIndex + 0],
                    mesh->mTextureCoords[2 * mesh->mMaterialIndex + 1]
                };*/
            }
            else
            {
                vertex.texCoord = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }
        //Now retrieve the corresponding vertex indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        return new Mesh(vertices, indices);//TODO 3: return with new Mesh of our own
    }

    void vkApplication::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            // the node object only contains indices to index the actual objects in the scene.
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes->push_back(processMesh(mesh));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(meshes, node->mChildren[i], scene);
        }
    }

    std::vector<Mesh*> vkApplication::loadMesh(const std::string& path)
    {
        std::vector<Mesh*> meshes;//TODO 1: create the container that will be returned by this function
        //read file with Assimp
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        //Check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Error loading model file \"%s\": \"%s\" ", path.c_str(), importer.GetErrorString());
            return meshes;
        }
        // retrieve the directory path of the filepath
        //std::string dir = path.substr(0, path.find_last_of('/'));
        //TODO 2: process Assimp's root node recursively
        processNode(&meshes, scene->mRootNode, scene);
        return meshes;
    }
    void vkApplication::initVulkan()
	{
		std::cout << "initVulkan!\n" << std::endl;
		m_SwapChain->createSwapChain(m_Device->getDevice(),m_Device->getPDevice(),m_Device->getSurface(),m_Instance->getWindow());
        m_SwapChain->createSwapChainImageViews(m_Device->getDevice());
        m_PipeLine->createRenderPass();
        m_Descriptor->createDescriptorSetLayout();
        m_PipeLine->createPipeLine(m_Descriptor->getDSlayout());
        m_Command->createCommandPool(m_Device->getDevice(), m_Device->getPDevice(), m_Device->getSurface());;
        m_Model->createDepthResources(m_Device->getPDevice(), m_Device->getDevice(), m_Command->getCommandPool(),m_Device->getGraphicsQueue());;
        m_PipeLine->createFrameBuffers();
        m_Model->createTextureImage(m_Device->getDevice(),m_Device->getPDevice(),m_Device->getGraphicsQueue(), "../textures/viking_room.png");
        m_Model->createTextureImageView(m_Device->getDevice());
        m_Model->createTextureSampler(m_Device->getPDevice(), m_Device->getDevice());
        auto meshes = loadMesh("../models/viking_room.obj");
        m_Meshes.push_back(meshes[0]);
        m_UniformBuffers->createVkBuffer<decltype(m_Meshes[0]->getVertices())>(m_Meshes[0]->getVertices(), m_VertexBuffer, m_VertexBufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       m_Device->getDevice(), m_Command->getCommandPool(), m_Device->getPDevice(), m_Device->getGraphicsQueue());
        m_UniformBuffers->createVkBuffer<decltype(m_Meshes[0]->getVertices())>(m_Meshes[0]->getVertices(), m_IndexBuffer, m_IndexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                      m_Device->getDevice(), m_Command->getCommandPool(), m_Device->getPDevice(), m_Device->getGraphicsQueue());
        m_UniformBuffers->createUniformBuffers(m_Device->getDevice(), m_Device->getPDevice(), m_MaxFramesInFlight);
        m_Descriptor->createDescriptorPool(m_MaxFramesInFlight);
        m_Descriptor->createDescriptorSets(m_UniformBuffers, m_MaxFramesInFlight,m_Model->m_Texture.m_TextureImageView, m_Model->m_Texture.m_TextureSampler);
        m_Command->createCommandBuffer(m_Device->getDevice(),m_MaxFramesInFlight);
        createSyncObjects();
	}
}

