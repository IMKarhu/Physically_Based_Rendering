#pragma once
#include "kRenderer.hpp"
#include "keyboardMovement.hpp"
#include "kEntity.hpp"

#include <memory>
#include <fstream>
#include <chrono>

namespace karhu
{


    class Application
    {
    public:
        Application();
        ~Application();

        
        void run();
        void update(float deltaTime);
        void renderEntities(kCamera& camera, uint32_t currentFrameIndex, uint32_t index);
    private:
        kRenderer m_Renderer{};
        //std::unique_ptr<kModel> m_Model;
        std::vector<kEntity> m_Entities;

        float m_DeltaTime = 0.0f;
        uint32_t m_CurrentFrame = 0;
        
        const std::vector<Vertex> m_Vertices = {
            {{-0.5f,-0.5f, 0.0f},{1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f},{0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f},{0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f},{1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint32_t> m_Indices = {
            0,1,2,2,3,0,
            4,5,6,6,7,4
        };
    };
} // namespace karhu
