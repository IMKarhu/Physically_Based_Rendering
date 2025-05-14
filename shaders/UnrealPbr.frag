#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragWorldPosition;

layout(set = 1, binding = 1) uniform sampler2D texSampler; //albedo
layout(set = 1, binding = 2) uniform sampler2D normalMap; //normal
layout(set = 1, binding = 3) uniform sampler2D metallicMap; //metallic and roughness
layout(set = 1, binding = 4) uniform sampler2D aoMap; //ao
layout(set = 1, binding = 5) uniform sampler2D emissiveMap; //emissive

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
float G_Smith(float NoV, float NoL, float a);
vec3 F_Schlick(vec3 f0, float HoV);
float Fd_Lambert();

void main()
{
 vec3 normal = getNormalFromMap();
    vec3 V = normalize(camera.cameraPosition - fragWorldPosition.xyz);
    vec3 albedo = texture(texSampler, fragUV).rgb;
    float metallic = texture(metallicMap, fragUV).b + camera.albedoNormalMetalRoughness.z;
    float roughness = texture(metallicMap, fragUV).g + camera.albedoNormalMetalRoughness.w;
//    float metallic = texture(metallicMap, fragUV).r + camera.albedoNormalMetalRoughness.z;
//    float roughness = texture(roughnessMap, fragUV).r + camera.albedoNormalMetalRoughness.w;
    if(roughness > 1.0)
    {
        roughness = 1.0;
    }
    vec3 ao = texture(aoMap, fragUV).rrr;
    float emissive = texture(emissiveMap, fragUV).r;
    
    //baseRefelectivity
    vec3 f0 = vec3(0.04);
    // Fresnel reflectance at normal incidence.
    f0 = mix(f0, albedo, metallic);
    
    //refletance equation
    vec3 Lo = vec3(0.0);


    //per light radiance, if we had more than one, we should calculate this for all the lights
    vec3 L = normalize(normalize(camera.lightPosition) - normalize(fragWorldPosition.xyz));
    vec3 H = normalize(V + L);
    float dist = length(normalize(camera.lightPosition) - normalize(fragWorldPosition.xyz));
    float attenuation = 1.0 / (dist *  dist);
    vec3 radiance = camera.lightColor.xyz * attenuation;

     //BRDF
    float NoV = max(dot(normal, V), 0.000001);
    float NoL = max(dot(normal, L), 0.000001);
    float HoV = max(dot(H, V), 0.0);
    float NoH = max(dot(normal, H), 0.0);

    float D = D_GGX(NoH, roughness);
    float G = G_Smith(NoV, NoL, roughness);
    vec3 F = F_Schlick(f0, HoV);

    vec3 specularBRDF = (F * G * D) / (4 * (NoL * NoV));

    //energy conservation, diffuse and specular light can't be above 1.0 (unles surface emits light)
    //Diffuse scattering.
    //Happens because light is refracted multiple times by dielectric medium.
    //metals reflect or absorb energy, diffuse is always zero.
    vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);
    //kD *= 1.0 - metallic;

    //Lambertian
    vec3 diffuseBRDF = kD * albedo;

    Lo += (diffuseBRDF + specularBRDF) * radiance * NoL;


    vec3 ambient = vec3(0.1) * albedo * ao;

    vec3 color = ambient + Lo;
    color *= Fd_Lambert();
    

    //hdr tonemapping
    color = color / (color + vec3(1.0));
    //gamma correction
    //color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}

float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float NoH2 = NoH * NoH;
    return a2 / (PI * pow((NoH2 * (a2 - 1)) + 1, 2));
}

float G_Smith(float NoV, float NoL, float a)
{
    float k = pow(a + 1, 2) / 8 ;
    float G1 = NoV / (NoV * (1 - k) + k);
    float G2 = NoL / (NoL * (1 - k) + k);
    return G1 * G2;
}

vec3 F_Schlick(vec3 f0, float HoV)
{
    return f0 + (vec3(1.0) - f0) * pow(2, ((-5.55473 * HoV) - 6.98316) * HoV);
}

float Fd_Lambert()
{
return 1.0 / PI;
}