#pragma once
#include "kTinyRenderer.hpp"
#include "kWindow.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kDescriptors.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{
    class vkglTFModel;
    class Application
    {
    public:
        Application();
        ~Application();
        void run();
        void buildVulkan();
        void update(float deltaTime);
    private:

        std::vector<vkglTFModel *> m_Models;
        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;
        const int m_MaxFramesInFlight = 2;

        engine::TinyRenderer* m_Renderer;
    };
} // namespace karhu
