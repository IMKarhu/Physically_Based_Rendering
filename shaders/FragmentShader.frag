#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragWorldPosition;

layout(set = 0, binding = 1) uniform sampler2D brdfLut;
layout(set = 0, binding = 2) uniform samplerCube irradianceCube;
layout(set = 0, binding = 3) uniform samplerCube prefilteredCube;

layout(set = 1, binding = 1) uniform sampler2D texSampler; //albedo
layout(set = 1, binding = 2) uniform sampler2D normalMap; //normal
layout(set = 1, binding = 3) uniform sampler2D metallicMap; //metallic and roughness
layout(set = 1, binding = 4) uniform sampler2D aoMap; //ao
layout(set = 1, binding = 5) uniform sampler2D emissive; //emissive

layout( push_constant, std140) uniform cameraConstants
{
    layout(offset = 64) vec3 cameraPosition;
    int offset;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 albedoNormalMetalRoughness;
} camera;

const float PI = 3.1415926535897932384626433832795;
const float EPSILON = 0.00001;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fragUV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fragWorldPosition.xyz);
    vec3 Q2  = dFdy(fragWorldPosition.xyz);
    vec2 st1 = dFdx(fragUV);
    vec2 st2 = dFdy(fragUV);

    vec3 N   = normalize(fragNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = normalize(cross(T, N));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float D_GGX(float NoH, float a);
vec3 F_Schlick(float u, vec3 f0);
vec3 F_SchlickR(float u, vec3 f0, float roughness);
float V_SmithGGXCorrelated(float NoV, float NoL, float a);
float Fd_Lambert();
vec3 prefilteredReflection(vec3 R, float roughness);
vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 albedo);

void main()
{
    vec3 normal = getNormalFromMap(); // normals
    vec3 V = normalize(camera.cameraPosition - fragWorldPosition.xyz);
    vec3 R = reflect(-V, normal);
    vec3 albedo = texture(texSampler, fragUV).rgb;
    float metallic = texture(metallicMap, fragUV).b;
    float roughness = texture(metallicMap, fragUV).g;
    float ao = texture(aoMap, fragUV).r;

    
    //baseRefelectivity
    vec3 f0 = vec3(0.04);
    // Fresnel reflectance at normal incidence.
    f0 = mix(f0, albedo, metallic);
    
    //refletance equation
    vec3 Lo = vec3(0.0);


    //per light radiance, if we had more than one, we should calculate this for all the lights
    vec3 L = normalize(normalize(camera.lightPosition) - normalize(fragWorldPosition.xyz));
    Lo += specularContribution(L, V, normal, f0, metallic, roughness, albedo);

    vec2 brdf = texture(brdfLut, vec2(max(dot(normal, V), 0.0), roughness)).rg;
    vec3 reflection = prefilteredReflection(R, roughness).rgb;
    vec3 irradiance = texture(irradianceCube, normal).rgb;
    
    // diffuse is based on irradiance
    vec3 diffuse = irradiance * albedo;

    vec3 F = F_SchlickR(max(dot(normal, V), 0.000001), f0, roughness);

    //specular reflectance
    vec3 specularRef = reflection * (F * brdf.x + brdf.y);

    vec3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    vec3 ambient = (kD * diffuse + specularRef);

    vec3 color = ambient + Lo;

    //hdr tonemapping
    //color = color / (color + vec3(1.0));
    //gamma correction
    //color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}

float D_GGX(float NoH, float a)
{
    //float a2 = a * a;
    //float f = (NoH * a2 - NoH) * NoH + 1.0;
    //return a2/ (PI * f * f); // DGGX(h,a) = a2/ PI((n*h)pow2 (a2-1)+1)pow2
    float alpha = a * a;
    float a2 = alpha * alpha;
    float denom = NoH * NoH * (a2 -1.0) +1.0;
    return (a2)/(PI * denom * denom);
}

vec3 F_Schlick(float u, vec3 f0)
{
    return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0); // Fresnel Schlick(v,h,f0,f90=1.0) = f0 + (f90 - f0)(1-v*h)power of 5
}

vec3 F_SchlickR(float u, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float ggxl = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float ggxv = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (ggxv + ggxl); //V(v,l,a) = 0.5/ n*l sqrt((n*v)pow2(1-a2)+a2) + n*v sqrt((n*l)pow2(1-a2)+a2
}

float Fd_Lambert()
{
    return 1.0 / PI;
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(prefilteredCube, R, lodf).rgb;
    vec3 b = textureLod(prefilteredCube, R, lodc).rgb;
    return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 albedo)
{
    vec3 H = normalize(V + L);
    float NoV = max(dot(N, V), 0.000001);
    float NoL = max(dot(N, L), 0.000001);
    float HoV = max(dot(H, V), 0.0);
    float NoH = max(dot(N, H), 0.0);

    vec3 lightColor = vec3(1.0);
    vec3 color = vec3(0.0); //color we return from this function

    if (NoL > 0.0)
    {
        float D = D_GGX(NoH, roughness); // Normal Distribution
        float G = V_SmithGGXCorrelated(NoV, NoL, roughness); //Geometric shadowing
        vec3 F = F_Schlick(NoV, F0); //Fresnel factor, reflectance depending on angle

        vec3 specular = D * F * G / (4.0 * NoL * NoV + 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        color += (kD * albedo / PI + specular) * NoL;
    }
    return color;
}
