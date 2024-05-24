#include "kApplication.hpp"

namespace karhu
{
    Application::Application()
    {
        m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);
    }

    Application::~Application()
    {

    }

    void Application::run()
    {
        while (!m_Window->shouldClose())
        {
            m_Window->pollEvents();
        }
    }

} // namespace karhu
