#pragma once
#include "kWindow.hpp"

#include <memory>

namespace karhu
{
    class Application
    {
    public:
        Application();
        ~Application();
        void run();

    private:
        std::unique_ptr<kWindow> m_Window;
    };
} // namespace karhu
