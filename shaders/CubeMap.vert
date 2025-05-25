#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 localPos;

void main()
{
    localPos = inPosition;
    gl_Position = m_Ubo.proj * m_Ubo.view * vec4(localPos, 1.0);
}