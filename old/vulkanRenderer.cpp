#include "vulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
	:m_Instance(VK_NULL_HANDLE),
    m_DebugMessenger(VK_NULL_HANDLE)
{
	
}

VulkanRenderer::~VulkanRenderer()
{
    cleanUpSwapChain();
    m_Texture->cleanUp(m_Device->getDevice());
    delete m_Texture;
    vkDestroyBuffer(m_Device->getDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device->getDevice(), m_VertexBufferMemory, nullptr);
    vkDestroyBuffer(m_Device->getDevice(), m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device->getDevice(), m_IndexBufferMemory, nullptr);
    delete m_Buffer;
    for (size_t i = 0; i < m_MaxFramesInFlight; i++)
    {
        vkDestroySemaphore(m_Device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device->getDevice(), m_InFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(m_Device->getDevice(), m_CommandPool, nullptr);
    delete m_PipeLine;
    delete m_SwapChain;
    delete m_Device;
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }
    vkDestroyInstance(m_Instance, nullptr);
}



void VulkanRenderer::Run()
{
	createInstance();
    setupDebugMessenger();
    m_Device = new Device(m_Instance,m_Window->getWindow(), enableValidationLayers, validationLayers);
    m_Device->createSurface();
    m_Device->pickPhysicalDevice();
    m_Device->createLogicalDevice();
    m_SwapChain = new SwapChain(m_Device);
    m_SwapChain->createSwapChain(m_Window->getWindow());
    m_SwapChain->createImageViews();
    m_Buffer = new Buffer(m_Device);
    m_Texture = new Texture(m_Buffer, m_SwapChain);
    m_PipeLine = new GraphicsPipeLine(m_Device, m_SwapChain, m_Texture);
    m_PipeLine->createRenderPass();
    m_Buffer->createDescriptorSetLayout();
    m_PipeLine->createPipeLine(m_Buffer->getDSLayout());
    createCommandPool();
    m_Texture->createDepthResources(m_Device->getPhysicalDevice(), m_Device->getDevice(), m_CommandPool);
    m_PipeLine->createFrameBuffers();
    m_Texture->createTextureImage("../textures/viking_room.png", m_Device->getDevice(), m_CommandPool);
    m_Texture->createTextureImageViews();
    m_Texture->createTextureSampler(m_Device->getPhysicalDevice(), m_Device->getDevice());
    auto meshes = loadMesh("../models/viking_room.obj");
    m_Meshes.push_back(meshes[0]);
    m_Buffer->createVkBuffer<decltype(m_Meshes[0]->getVertices())>(m_Meshes[0]->getVertices(), m_VertexBuffer, m_VertexBufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        m_Device->getDevice(), m_CommandPool);
    m_Buffer->createVkBuffer<decltype(m_Meshes[0]->getIndices())>(m_Meshes[0]->getIndices(), m_IndexBuffer, m_IndexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        m_Device->getDevice(), m_CommandPool);
    m_Buffer->createUniformBuffers(m_MaxFramesInFlight);
    m_Buffer->createDescriptorPool(m_MaxFramesInFlight);
    m_Buffer->createDescriptorSets(m_MaxFramesInFlight, m_Texture->getTextureImageView(), m_Texture->getTextureSampler());
    createCommandBuffers();
    createSyncObjects();

	while (!m_Window->windowShouldClose())
	{
		m_Window->pollEvents();
        drawFrame();
	}
    vkDeviceWaitIdle(m_Device->getDevice());
}

void VulkanRenderer::createInstance()
{
    if (enableValidationLayers && !checkValidationSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanRenderer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = m_Device->findQueueFamilies(m_Device->getPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_Device->getDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanRenderer::createCommandBuffers()
{
    m_CommandBuffers.resize(m_MaxFramesInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

    if (vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo,m_CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandbuffer, uint32_t imageIndex)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f,0.0f,1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandbuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_PipeLine->getRenderPass();
    renderPassInfo.framebuffer = m_PipeLine->getFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipeLine->getPipeLine());

    VkBuffer vertexBuffers[] = { m_VertexBuffer};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandbuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_SwapChain->getSwapChainExtent().width;
    viewport.height = (float)m_SwapChain->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_SwapChain->getSwapChainExtent();
    vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipeLine->getPipeLineLayout(), 0, 1,
        &m_Buffer->getDescriptorSets()[m_CurrentFrame],0, nullptr);

    vkCmdDrawIndexed(commandbuffer, static_cast<uint32_t>(m_Meshes[0]->getIndices().size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandbuffer);

    if (vkEndCommandBuffer(commandbuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::createSyncObjects()
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

void VulkanRenderer::drawFrame()
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

    m_Buffer->updateUniformBuffers(m_CurrentFrame, m_SwapChain->getSwapChainExtent());
    vkResetFences(m_Device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
    recordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->getFrameBufferResize())
    {
        m_Window->setFrameBufferResize(false);
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
}

void VulkanRenderer::recreateSwapChain()
{
    int width = 0, height = 0;
    //glfwGetFramebufferSize(m_Window->getWindow(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_Window->getWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_Device->getDevice()); /* We shouldnt touch on resources that may still be in use, thus we wait. */
    cleanUpSwapChain();
    m_SwapChain->createSwapChain(m_Window->getWindow());
    m_SwapChain->createImageViews(); /* Image views need to be recreated because they are directly based on the swapchain images. */
    m_Texture->createDepthResources(m_Device->getPhysicalDevice(), m_Device->getDevice(), m_CommandPool);
    m_PipeLine->createFrameBuffers(); /* Same as image views. */
}

void VulkanRenderer::cleanUpSwapChain()
{
    m_Texture->cleanUpForRecreation(m_Device->getDevice());
    for (size_t i = 0; i < m_PipeLine->getFrameBuffers().size(); i++) {
        vkDestroyFramebuffer(m_Device->getDevice(), m_PipeLine->getFrameBuffers()[i], nullptr);
    }
    for (size_t i = 0; i < m_SwapChain->getSwapChainImageViews().size(); i++) {
        vkDestroyImageView(m_Device->getDevice(), m_SwapChain->getSwapChainImageViews()[i], nullptr);
    }
    vkDestroySwapchainKHR(m_Device->getDevice(), m_SwapChain->getSwapChain(), nullptr);
}

void VulkanRenderer::createVertexBuffers()
{
    VkDeviceSize bufferSize = sizeof(m_Meshes[0]->getVertices()[0]) * m_Meshes[0]->getVertices().size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(m_Device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_Meshes[0]->getVertices().data(), (size_t)bufferSize);
    vkUnmapMemory(m_Device->getDevice(), stagingBufferMemory);
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VertexBuffer, m_VertexBufferMemory);

    copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

    vkDestroyBuffer(m_Device->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_Device->getDevice(), stagingBufferMemory, nullptr);
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequiremenets;
    vkGetBufferMemoryRequirements(m_Device->getDevice(), buffer, &memRequiremenets);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequiremenets.size;
    memAllocInfo.memoryTypeIndex = findMemoryType(memRequiremenets.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device->getDevice(), &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device->getDevice(), buffer, bufferMemory, 0);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_Device->getGraphicsQueue());

    vkFreeCommandBuffers(m_Device->getDevice(), m_CommandPool, 1, &commandBuffer);
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find memory type!");
}

Mesh* VulkanRenderer::processMesh(aiMesh* mesh)
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
            vertex.texCoord = {
                mesh->mTextureCoords[2 * mesh->mMaterialIndex + 0],
                mesh->mTextureCoords[2 * mesh->mMaterialIndex + 1]
            };
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

void VulkanRenderer::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene)
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

std::vector<Mesh*> VulkanRenderer::loadMesh(const std::string& path)
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

bool VulkanRenderer::checkValidationSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanRenderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
