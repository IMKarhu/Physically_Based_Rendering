#version 450
//#extension GL_KHR_vulkan_glsl: enable

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
layout(location = 4) in vec3 inTangent;

layout(location = 0) out vec3 fragColors;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragWorldPosition;
layout(location = 4) out vec3 fragTangent;

void main()
{

    gl_Position = m_Ubo.proj * m_Ubo.view * obj.model * vec4(inPosition, 1.0);

    fragWorldPosition = vec3(obj.model * vec4(inPosition, 1.0));
    fragColors = inColor;
    //fragNormal = mat3(obj.model) * inNormal;
    // fragNormal = inNormal;
    fragNormal = mat3(transpose(inverse(m_Ubo.view * obj.model))) * inNormal; // normals in view space
    // fragNormal = mat3(transpose(inverse(obj.model))) * inNormal; //world space
    fragUV = inUV;
    fragTangent = inTangent;
}
