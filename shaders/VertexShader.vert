#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    //mat4 model;
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(set = 1, binding = 0) uniform ObjetBuffer
{
    mat4 model;
} m_ObjBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColors;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec4 fragWorldPosition;

void main()
{
    fragWorldPosition =  m_ObjBuffer.model * vec4(inPosition, 1.0);
    gl_Position = m_Ubo.proj * m_Ubo.view * fragWorldPosition;

    
    fragColors = inColor;
    fragNormal = mat3(m_ObjBuffer.model) * inNormal; // normals in world space
    fragUV = inUV;
}