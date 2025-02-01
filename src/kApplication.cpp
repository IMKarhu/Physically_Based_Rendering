#include "kApplication.hpp"
#include "kCamera.hpp"


namespace karhu
{
    Application::Application()
    {

    }

    Application::~Application()
    {
        delete m_Model;
    }

    void Application::run()
    {
        //m_Model = std::make_unique<kModel>(m_Renderer.getDevice(), m_Vertices, m_Indices, m_Renderer.getCommandPool());
        m_Model = new kModel(m_Renderer.getDevice(), m_Vertices, m_Indices, m_Renderer.getCommandPool());
        update(m_DeltaTime);
    }

    void Application::update(float deltaTime)
    {
        kCamera m_Camera{};
        while (!m_Renderer.getWindowShouldclose())
        {
            m_Renderer.windowPollEvents();
            uint32_t imageIndex = 0;
            m_Renderer.beginFrame(m_CurrentFrame, imageIndex);
            m_Renderer.recordCommandBuffer(m_Model, m_Indices,  m_CurrentFrame, imageIndex);
            m_Renderer.updateUBOs(imageIndex, m_Camera);
            m_Renderer.endFrame(m_CurrentFrame, imageIndex);
            //drawFrame();
        }
        vkDeviceWaitIdle(m_Renderer.getDevice().m_Device);
    }
} // namespace karhu
