#include "kApplication.hpp"
#include "kBuffer.hpp"
#include "kModel.hpp"




namespace karhu
{
    Application::Application()
    {
        m_Renderer = new engine::TinyRenderer();
    }

    Application::~Application()
    {
        printf("app destroyed\n");
    }

    void Application::run()
    {
        buildVulkan();
        update(m_DeltaTime);
    }

    void Application::buildVulkan()
    {
        m_Models.push_back(new vkglTFModel(m_Renderer->getDeviceStruct()));
        m_Renderer->buildVulkan(m_MaxFramesInFlight, m_Models);
    }

    void Application::update(float deltaTime)
    {
        for (const auto& model : m_Models)
        {
            m_Renderer->update(m_CurrentFrame, model);
        }
    }
} // namespace karhu
