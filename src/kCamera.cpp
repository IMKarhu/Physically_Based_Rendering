#include "kCamera.hpp"

namespace karhu {
    Camera::Camera(GLFWwindow *window, uint32_t width, uint32_t height)
       : m_Window(window), m_Width(width), m_Height(height),
         m_Position(glm::vec3(0.0f, 0.0f, 0.0f)),
         m_Up(glm::vec3(0.0f, 0.0f, 1.0f)), m_Eulers(glm::vec3(0.0f, 1.0f, 0.0f)),
         m_Forward(glm::vec3(0.0f, 0.0f, 0.0f)),
         m_Right(glm::vec3(0.0f, 0.0f, 0.0f)), m_ViewMatrix(glm::mat4(1.0f)),
         m_ModelMatrix(glm::mat4(1.0f)), m_Perspectivematrix(glm::mat4(1.0f)) {
     // setMatrices();
    }

    Camera::~Camera() {}

    void Camera::setMatrices()
    {
        m_Perspectivematrix = glm::perspective(
      glm::radians(45.0f), m_Width / (float)m_Height, 0.1f, 100.0f);
        m_ViewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), m_Position, m_Up);
        m_ModelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                              glm::vec3(0.0f, 0.0f, 1.0f));
    }

    void Camera::update(float deltaTime)
    {
    setMatrices();
    float yaw = glm::radians(m_Eulers.y);
    float pitch = glm::radians(m_Eulers.z);

    m_Forward = {glm::cos(yaw) * glm::cos(pitch), glm::sin(pitch),
                 glm::sin(yaw) * glm::cos(pitch)};

    m_Right = glm::normalize(glm::cross(m_Forward, m_Up));

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
      m_Position += m_Forward * m_CameraSpeed * deltaTime;
      printf("heelo");
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
      m_Position -= m_Forward * m_CameraSpeed * deltaTime;
      printf("hello s");
    }

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    //printf("m_position %f, %f, %f\n", m_Position.x, m_Position.y, m_Position.z);
    //printf("delta %f", deltaTime);
    }
} // namespace karhu
