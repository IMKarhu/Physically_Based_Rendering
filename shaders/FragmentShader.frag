#version 450
//#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragWorldPosition;
layout(location = 4) in vec3 fragTangent;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} m_Ubo;

layout(set = 1, binding = 0) uniform ObjBuffer
{
    mat4 model;
} m_Obj;

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

layout(set = 1, binding = 1) uniform sampler2D texSampler; //albedo
layout(set = 1, binding = 2) uniform sampler2D normalMap; //normal
layout(set = 1, binding = 3) uniform sampler2D metallicMap; //metallic and roughness
layout(set = 1, binding = 4) uniform sampler2D aoMap; //ao
layout(set = 1, binding = 5) uniform sampler2D emissiveMap; //emissive

const vec3 lPos = vec3(0.0, 10.0, 0.0);
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

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fragUV).xyz * 2.0 - 1.0;

    vec3 N   = normalize(fragNormal);
    vec3 T  = normalize(fragTangent.xyz);
    vec3 B  = normalize(cross(T, N));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float D_GGX(float NoH, float a);
vec3 F_Schlick(float u, vec3 f0);
vec3 F_SchlickR(float u, vec3 f0, float roughness);
float V_SmithGGXCorrelated(float NoV, float NoL, float a);
vec3 Fd_Lambert();
vec3 prefilteredReflection(vec3 R, float roughness);
vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness, vec3 albedo);
vec3 specularContributionNoIbl(vec3 L, vec3 V, vec3 N, vec3 f0, vec3 albedo, vec3 ao, float metallic, float roughness);


void main()
{
    vec3 normal = getNormalFromMap(); // normals
    vec3 albedo = texture(texSampler, fragUV).rgb;
    float metallic = texture(metallicMap, fragUV).b;
    float roughness = texture(metallicMap, fragUV).g;
    vec3 ao = texture(aoMap, fragUV).rrr;
    vec3 emissive = texture(emissiveMap, fragUV).rgb;

    // vec3 camPos = vec3(m_Obj.model * vec4(camera.cameraPosition, 1.0));
    vec3 V = normalize(camera.cameraPosition - fragWorldPosition);


    //baseRefelectivity
    vec3 f0 = vec3(0.04);
    // Fresnel reflectance at normal incidence.
    f0 = mix(f0, albedo, metallic);
    vec3 Lo = vec3(0.0);

    if(camera.offset == 3.0) //PBR
    {
        // vec3 lightPos = vec3(m_Obj.model * vec4(camera.lightPosition, 1.0));
        // vec3 L = normalize(normalize(camera.lightPosition) - normalize(fragWorldPosition.xyz));
        vec3 L = normalize(normalize(lPos) - normalize(fragWorldPosition.xyz));

        float rroughness = roughness;
        Lo += specularContributionNoIbl(L, V, normal, f0, albedo, ao, metallic, rroughness);

        //vec3 color = albedo * ao;
        vec3 ambient = vec3(0.1) * albedo * ao;
        vec3 color = Lo;

        //hdr tonemapping
        color = color / (color + vec3(1.0));
        //gamma correction
        color = pow(color, vec3(1.0/2.2));

        outColor = vec4(color, 1.0);
    }
    else if(camera.offset == 2.0) // PBR+IBL
    {
        vec3 R = reflect(V, normal);
        //per light radiance, if we had more than one, we should calculate this for all the lights
        // vec3 lightPos = vec3(m_Obj.model * vec4(camera.lightPosition, 1.0));
        // vec3 L = normalize(normalize(camera.lightPosition) - normalize(fragWorldPosition.xyz));
        vec3 L = normalize(normalize(lPos) - normalize(fragWorldPosition.xyz));
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

        vec3 color = ambient + Lo + emissive;

        //hdr tonemapping
         color = Uncharted2Tonemap(color * 4.5); // 4.5 is exposure
         color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
        //gamma correction
        //color = pow(color, vec3(1.0/2.2)); // 2.2 is gamma value

        outColor = vec4(color, 1.0);
    }
    else if(camera.offset == 1.0) // PHONG
    {
        // vec3 L = normalize(camera.lightPosition - fragWorldPosition);
        vec3 L = normalize(normalize(lPos) - normalize(fragWorldPosition.xyz));

        float specularStrength = 0.5;
        float ambientStrength = 0.1;

        vec3 ambient = ambientStrength * camera.lightColor.xyz;

        vec3 R = reflect(-V, normal);
        float specular = pow(max(dot(V, R), 0.0), 32);
        vec3 specularComponent = specularStrength * specular * camera.lightColor.xyz;

        float diffuse = max(dot(normal, L), 0.0);
        vec3 diffuseComponent = diffuse * camera.lightColor.xyz;

        vec3 color = (ambient + diffuseComponent + specularComponent) * albedo;
        outColor = vec4(color, 1.0);
    }
    else if(camera.offset == 4.0) // Blinn-Phong
    {
        // vec3 L = normalize(camera.lightPosition - fragWorldPosition);
        vec3 L = normalize(normalize(lPos) - normalize(fragWorldPosition.xyz));
        vec3 H = normalize(L + V);
        float specularStrength = 0.5;
        float ambientStrength = 0.1;

        vec3 ambient = ambientStrength * camera.lightColor.xyz;

        // vec3 R = reflect(-V, normal);
        float specular = pow(max(dot(normal, H), 0.0), 32);
        vec3 specularComponent = specular * camera.lightColor.xyz;

        float diffuse = max(dot(normal, L), 0.0);
        vec3 diffuseComponent = diffuse * camera.lightColor.xyz;

        vec3 color = (ambient + diffuseComponent + specularComponent) * albedo;
        outColor = vec4(color, 1.0);

    }
    else
    {
        outColor = vec4(albedo, 1.0);
    }
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

vec3 Fd_Lambert(vec3 albedo)
{
    return (albedo / PI);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 4.0;
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

vec3 specularContributionNoIbl(vec3 L, vec3 V, vec3 N, vec3 f0, vec3 albedo, vec3 ao, float metallic, float roughness)
{
    vec3 H = normalize(V + L);
                    
    //BRDF
    // float NoV = clamp(dot(N, V), 0.0, 1.0);
    // float NoL = clamp(dot(N, L), 0.0, 1.0);
    // float HoV = clamp(dot(H, V), 0.0, 1.0);
    // float NoH = clamp(dot(N, H), 0.0, 1.0);
    float NoV = max(dot(N, V), 0.000001);
    float NoL = max(dot(N, L), 0.000001);
    float HoV = max(dot(H, V), 0.0);
    float NoH = max(dot(N, H), 0.0);

    vec3 color = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 kS = vec3(0.0);
    vec3 kD = vec3(0.0);
    if(NoL > 0.0)
    {
        //float rroughness = max(0.05, roughness);
        float D = D_GGX(NoH, roughness); //Normal distribution for specular BRDF.  value between 0 and 1
        float G = V_SmithGGXCorrelated(NoV, NoL, roughness); //Geometric attenuation for specular BRDF. value between 0 and 1
        vec3 F = F_Schlick(HoV, f0); //Fresnel term for direct lighting. RGB values also between 0 and 1

        specular = D * F * G / (4.0 * NoL * NoV + 0.001);

       //kS = F; //specular reflection ratio
       //kD = 1.0 - kS;
       //kD *= 1.0 - metallic; // no diffuse in metals
        kD = mix(vec3(1.0) - F, vec3(0.0), metallic);

        vec3 diffuse = Fd_Lambert(albedo);
        color = (kD * diffuse + specular) * NoL * camera.lightColor.xyz;
    }

    return color;
}
