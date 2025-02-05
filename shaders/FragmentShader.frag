#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;


const vec3 Directional_Light = normalize(vec3(1.0,3.0,-1.0));

void main()
{

    float lightIntensity = max(dot(fragNormal, Directional_Light), 0);

    outColor = vec4(fragColor * lightIntensity, 1.0);
}