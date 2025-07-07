#version 450
//#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragWorldPosition;
layout(location = 4) in vec3 fragTangent;

layout( push_constant, std140) uniform cameraConstants
{
    layout(offset = 64) vec3 cameraPosition;
    int offset;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 albedoNormalMetalRoughness;
} camera;

layout(set = 0, binding = 1) uniform sampler2D brdfLut;
layout(set = 0, binding = 2) uniform samplerCube irradianceCube;
layout(set = 0, binding = 3) uniform samplerCube prefilteredCube;

const float PI = 3.1415926535897932384626433832795;
const float EPSILON = 0.00001;

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

float D_GGX(float NoH, float a);
vec3 F_Schlick(float u, vec3 f0);
vec3 F_SchlickR(float u, vec3 f0, float roughness);
float V_SmithGGXCorrelated(float NoV, float NoL, float a);
float Fd_Lambert();
vec3 prefilteredReflection(vec3 R, float roughness);
vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 albedo);

void main()
{
    vec3 normal = normalize(fragNormal); // normals
    vec3 albedo = fragColor;
    float metallic = 0.1;
    float roughness = 0.5;
    vec3 ao = vec3(1.0);

    vec3 V = normalize(camera.cameraPosition - fragWorldPosition.xyz);
    vec3 R = reflect(-V, normal);


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

    vec3 F = F_SchlickR(max(dot(normal, V), 0.0), f0, roughness);

    //specular reflectance
    vec3 specularRef = reflection * (F * brdf.x + brdf.y);

    vec3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    vec3 ambient = (kD * diffuse + specularRef) * ao;

    vec3 color = ambient + Lo;

    //hdr tonemapping
    // color = color / (color + vec3(1.0));
    //
    color = Uncharted2Tonemap(color * 4.5); // 4.5 is exposure
    color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
    //gamma correction
    // color = pow(color, vec3(1.0/2.2)); // 2.2 is gamma value

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
    //float a2 = a * a;
    //float ggxl = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    //float ggxv = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    //return 0.5 / (ggxv + ggxl); //V(v,l,a) = 0.5/ n*l sqrt((n*v)pow2(1-a2)+a2) + n*v sqrt((n*l)pow2(1-a2)+a2
    float r = a * a;
    float k = (r*r) / 8.0;
    float gl = NoL / (NoL * (1.0 - k) + k);
    float gv = NoV / (NoV * (1.0 - k) + k);
    return gl * gv;
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
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float HoV = clamp(dot(H, V), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);

    vec3 lightColor = vec3(1.0); // constant light color
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
