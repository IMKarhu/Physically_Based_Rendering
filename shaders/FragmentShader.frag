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
} camera;

const float PI = 3.1415926535897932384626433832795;
const vec3 Directional_Light = normalize(vec3(1.0,3.0,1.0));
const float AMBIENT = 0.01;

float NDF_GGX(vec3 N, vec3 H, float roughness);
float SpecularG_SmithGGXCorrelated(vec3 N, vec3 V, vec3 L, float roughness);
vec3 Schlick(float u, vec3 f0);
float Lambertian();
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
    //vec3 N = normalize();
    //float roughness = 0.5;
    vec3 normal = getNormalFromMap();

    vec3 V = normalize(camera.cameraPosition - fragWorldPosition.xyz);

    vec3 albedo = pow(texture(texSampler, fragUV).rgb, vec3(2.2));
    float metallic = texture(metallicMap, fragUV).r;
    float roughness = texture(roughnessMap, fragUV).r;
    float ao = texture(roughnessMap, fragUV).r;
    //normal = normalize(normal * 2.0 - 1.0);
    
    //baseRefelectivity
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);
    
    //refletance equation
    vec3 Lo = vec3(0.0);

    //per light radiance, if we had more than one, we should calculate this for all the lights
    vec3 L = normalize(normalize(camera.lightPosition) - fragWorldPosition.xyz);
    vec3 H = normalize(V + L);
    float dist = length(normalize(camera.lightPosition) - fragWorldPosition.xyz);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = vec3(1.0, 1.0, 1.0) * attenuation;


    //BRDF
    float NoV = max(dot(normal, V), 0.000001);
    float NoL = max(dot(normal, L), 0.000001);
    float HoV = max(dot(H, V), 0.0);
    float NoH = max(dot(normal, H), 0.0);

    float D = D_GGX(NoH, roughness); // value between 0 and 1
    float G = V_SmithGGXCorrelated(NoV, NoL, roughness); // value between 0 and 1
    vec3 F = F_Schlick(HoV, f0); //RGB values also between 0 and 1

    vec3 SpecularBRDF = D * G * F;
    SpecularBRDF /= 4.0 * NoV * NoL;

    //energy conservation, diffuse and specular light can't be above 1.0 (unles surface emits light)
    vec3 kD = vec3(1.0) - F;

    kD *= 1.0 - metallic;

    Lo += (kD * albedo / PI + SpecularBRDF) * radiance * NoL;

    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));

    color = pow(color, vec3(1.0/2.0));

    outColor = vec4(color, 1.0);

   // float NDF = NDF_GGX(normal, H, roughness);
  //  float G = SpecularG_SmithGGXCorrelated(normal, V, L, roughness);
   // vec3 f = Schlick(max(dot(H, V), 0.0), f0);

   // vec3 numerator    = NDF * G * f; 
      //  float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
     //   vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
      //  vec3 kS = f;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
       // vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
       // kD *= 1.0 - metallic;	  

        // scale light by NdotL
      //  float NdotL = max(dot(normal, L), 0.0);        

        // add to outgoing radiance Lo
      //  Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

        // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
   // vec3 ambient = vec3(0.03) * albedo * ao;
    
   // vec3 color = ambient + Lo;

    // HDR tonemapping
   // color = color / (color + vec3(1.0));
    // gamma correct
  //  color = pow(color, vec3(1.0/2.2)); 

   // outColor = vec4(color, 1.0);


    //float lightIntensity = AMBIENT + max(dot(fragNormal, Directional_Light), 0);

    //outColor = texture(texSampler, fragUV) * lightIntensity;
}

float NDF_GGX(vec3 N, vec3 H, float roughness)
{
    float roughness2 = roughness * roughness;
    float NdotH = max(dot(N,H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = roughness2;
    float denom = (NdotH2 * (roughness2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float SpecularG_SmithGGXCorrelated(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 Schlick(float u, vec3 f0)
{
    float f = pow(1.0 - u, 5.0);
    vec3 result = f + f0 * (1.0 - f);
    return result;
}

float Lambertian()
{
    return 1.0 / PI;
}

float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2/ (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0)
{
return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float ggxl = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float ggxv = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
return 0.5 / (ggxv + ggxl);
}

float Fd_Lamber()
{
return 1.0 / PI;
}
