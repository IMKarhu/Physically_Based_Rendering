#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 localPos;

layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 1) uniform samplerCube cubeMap;
layout(set = 1, binding = 2) uniform Params
{
    float exposure;
    float gamma;
} params;

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

void main()
{
    vec3 color = texture(cubeMap, localPos).rgb;

    color = Uncharted2Tonemap(color * params.exposure);
    color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));

    //Gamma correction
    color = pow(color, vec3(1.0f / params.gamma));

    fragColor = vec4(color, 1.0);
}
