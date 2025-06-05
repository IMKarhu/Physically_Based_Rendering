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

    mat4 rotView = mat4(mat3(m_Ubo.view)); // this removes translation from view matrice
    vec4 position = m_Ubo.proj * rotView * vec4(localPos, 1.0);

    gl_Position = position.xyww;
}