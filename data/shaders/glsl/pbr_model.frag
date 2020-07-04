#version 410 core
#define PI 3.14159265358979323846264338327950288
#define MAX_LIGHT_COUNT 4

struct VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

in VS_OUT vs_pass;

layout (location = 0) out vec4 out_color;

struct Light
{
    vec3 position;
    vec3 color;
};

uniform Light u_lights[MAX_LIGHT_COUNT];

uniform vec4 u_view_pos;

/// IBL
uniform samplerCube u_irradiance_map;
uniform samplerCube u_specular_map;
uniform sampler2D u_brdf_lut;
uniform sampler2D u_albedo;
uniform sampler2D u_roughness;
uniform sampler2D u_metallic;
uniform int u_debug;

// NDF(n, h, alpha) = alpha^2 / (pi * ((n dot h)^2 * (alpha^2 - 1) + 1)^2)
float DistributionGGX(in vec3 N, in vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float nom = a2;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    // if roughness = 0, NDF = 0,
    // if roughness = 1, NDF = 1 / pi
    return nom / denom;
}

// G_SchlickGGX(n, v, k) = dot(n, v) / (dot(n, v)(1 - k) + k)
// k is a remapping of alpha
float G_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = G_SchlickGGX(NdotV, roughness);
    float ggx2 = G_SchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel approximation F0 + (1 - F0) * (1 - (cosine))^5
vec3 FresnelSchlick(float cosTheta, in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, in vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness) - F0, vec3(0.0))) * pow(1.0 - cosTheta, 5.0);
}

const float ao = 1.0;

void main()
{
    // variables
    vec3 position = vs_pass.position;

    vec3 albedo = texture(u_albedo, vs_pass.uv).rgb;
    float metallic = texture(u_metallic, vs_pass.uv).r;
    float roughness = texture(u_roughness, vs_pass.uv).r;

    vec3 N = normalize(vs_pass.normal);
    vec3 V = normalize(u_view_pos.xyz - position);
    vec3 R = reflect(-V, N);

    if (u_debug == 1)
    {
        out_color = vec4(albedo, 1.0);
        return;
    }
    else if (u_debug == 2)
    {
        out_color = vec4(N, 1.0);
        return;
    }
    else if (u_debug == 3)
    {
        out_color = vec4(vec3(metallic), 1.0);
        return;
    }
    else if (u_debug == 4)
    {
        out_color = vec4(vec3(roughness), 1.0);
        return;
    }

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);
#if 0
    for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        // calculate per-light radiance
        vec3 delta = u_lights[i].position - position;
        vec3 L = normalize(delta);
        vec3 H = normalize(V + L);
        float distance = length(delta);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = u_lights[i].color * attenuation;

        // Cook-Torracne BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 nom = NDF * G * F;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float denom = 4.0 * NdotV * NdotV;
        vec3 specular = nom / max(denom, 0.001); // prevent devide by 0

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
#endif

    // image based ambient lighting
    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(u_irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;

    // sample both pre-filtered map and BRDF lut and combine then together
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_specular_map, R, roughness * MAX_REFLECTION_LOD).rgb;
    float reflectPower = max(dot(N, V), 0.0);
    vec2 brdfUV = vec2(reflectPower, 1.0 - roughness); // flip
    vec2 brdf = texture(u_brdf_lut, brdfUV).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    out_color = vec4(color, 1.0);
}

