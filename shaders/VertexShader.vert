#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(set = 1, binding = 0) uniform ObjBuffer
{
    mat4 model;
} m_Obj;

layout( push_constant ) uniform objPushConstant
{
    mat4 model;
} obj;

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
    fragWorldPosition =  obj.model * vec4(inPosition, 1.0);
    gl_Position = m_Ubo.proj * m_Ubo.view * fragWorldPosition;

    //normalworldspace
    vec3 nws = normalize(mat3(obj.model) * inNormal);
    
    fragColors = inColor;
    fragNormal = nws;
    fragUV = inUV;
}