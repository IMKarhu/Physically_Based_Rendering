#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColors;
layout(location = 1) out vec3 fragNormal;

const vec3 Directional_Light = normalize(vec3(1.0,3.0,-1.0));


void main()
{
    gl_Position = m_Ubo.proj * m_Ubo.view * m_Ubo.model *  vec4(inPosition, 1.0);

    //normalworldspace
    vec3 nws = normalize(mat3(m_Ubo.model) * inNormal);

    float lightIntensity = max(dot(nws, Directional_Light), 0);

    fragColors = inColor;
    fragNormal = nws;
}