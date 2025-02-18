#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragWorldPosition;

layout(binding = 1) uniform sampler2D texSampler; //albedo
layout(binding = 2) uniform sampler2D normalMap; //normal
layout(binding = 3) uniform sampler2D metallicMap; //metallic and roughness
layout(binding = 4) uniform sampler2D roughnessMap; //ao
layout(binding = 5) uniform sampler2D aoMap; //emissive

layout( push_constant ) uniform cameraConstants
{
    vec3 cameraPosition;
    int offset;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 albedoNormalMetalRoughness;
} camera;

const float PI = 3.1415926535897932384626433832795;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fragUV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fragWorldPosition.xyz);
    vec3 Q2  = dFdy(fragWorldPosition.xyz);
    vec2 st1 = dFdx(fragUV);
    vec2 st2 = dFdy(fragUV);

    vec3 N   = normalize(fragNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float D_GGX(float NoH, float a);
vec3 F_Schlick(float u, vec3 f0);
float V_SmithGGXCorrelated(float NoV, float NoL, float a);
float Fd_Lamber();

void main()
{
    vec3 normal = getNormalFromMap();
    //vec3 normal = texture(normalMap, fragUV).rgb;

    vec3 V = normalize(camera.cameraPosition - fragWorldPosition.xyz);

    vec3 albedo = texture(texSampler, fragUV).rgb;
    float metallic = texture(metallicMap, fragUV).r + camera.albedoNormalMetalRoughness.z;
    float roughness = texture(roughnessMap, fragUV).r + camera.albedoNormalMetalRoughness.w;
    if(roughness > 1.0)
    {
        roughness = 1.0;
    }
    float ao = texture(aoMap, fragUV).r;
    //normal = normalize(normal * 2.0 - 1.0);
    
    //baseRefelectivity
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);
    
    //refletance equation
    vec3 Lo = vec3(0.0);

//    for( int i = 0; i < 1; i++)
//    {
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
                
        float D = D_GGX(NoH, roughness); // value between 0 and 1
        float G = V_SmithGGXCorrelated(NoV, NoL, roughness); // value between 0 and 1
        vec3 F = F_Schlick(HoV, f0); //RGB values also between 0 and 1
                
        vec3 SpecularBRDF = D * G * F;
        
        //energy conservation, diffuse and specular light can't be above 1.0 (unles surface emits light)
        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 - metallic;
        Lo += (kD * albedo / PI + SpecularBRDF) * radiance * NoL;
//    }
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    //hdr tonemapping
    color = color / (color + vec3(1.0));
    //gamma correction
    //color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}

float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2/ (PI * f * f); // DGGX(h,a) = a2/ PI((n*h)pow2 (a2-1)+1)pow2
}

vec3 F_Schlick(float u, vec3 f0)
{
return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0); // Fresnel Schlick(v,h,f0,f90=1.0) = f0 + (f90 - f0)(1-v*h)power of 5
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float ggxl = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float ggxv = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
return 0.5 / (ggxv + ggxl); //V(v,l,a) = 0.5/ n*l sqrt((n*v)pow2(1-a2)+a2) + n*v sqrt((n*l)pow2(1-a2)+a2
}

float Fd_Lamber()
{
return 1.0 / PI;
}
