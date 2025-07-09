#version 450
//#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 4) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} m_Ubo;
layout(set = 1, binding = 0) uniform Params
{
    mat4 view;
    mat4 proj;
    float exposure;
    float gamma;
} params;


layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 localPos;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    localPos = inPosition;
    //localPos.xy *= -1.0;

    mat4 rotView = mat4(mat3(m_Ubo.view)); // this removes translation from view matrice
    gl_Position = m_Ubo.proj * m_Ubo.view * vec4(inPosition.xyz, 1.0);
}
