#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColors;
layout(location = 1) out vec2 fragTexCoord;


void main()
{
    gl_Position = m_Ubo.proj * m_Ubo.view * m_Ubo.model * vec4(inPosition, 1.0);
    fragColors = inColor;
    fragTexCoord = inTexCoord;
}