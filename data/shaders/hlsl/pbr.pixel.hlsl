#define PI 3.14159265358979323846264338327950288
#define MAX_LIGHT_COUNT 4

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION; // pack metallic in w component
    float4 normal : NORMAL; // pack roughess in w component
};

struct Light
{
    float4 position;
    float4 color;
};

cbuffer LightBuffer: register(b0)
{
    Light g_lights[MAX_LIGHT_COUNT];
};

cbuffer PerFrameBuffer : register(b1)
{
    float3 view_position;
    int debug;
};

Texture2D brdfLut : register(t1);
TextureCube specularMap : register(t2);
TextureCube irradianceMap : register(t3);

SamplerState g_sampler : register(s0);
SamplerState g_samplerLod : register(s1);

// NDF(n, h, alpha) = alpha^2 / (pi * ((n dot h)^2 * (alpha^2 - 1) + 1)^2)
float DistributionGGX(in float3 N, in float3 H, float roughness)
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
    // return nom / max(denom, 0.001); // prevent divide by zero for roughness = 0.0 and NdotH = 1.0
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

float GeometrySmith(in float3 N, in float3 V, in float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = G_SchlickGGX(NdotV, roughness);
    float ggx2 = G_SchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel approximation F0 + (1 - F0) * (1 - (cosine))^5
float3 FresnelSchlick(float cosTheta, in float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, in float3 F0, float roughness)
{
    float3 oneMinusRoughness = float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness) - F0;
    return F0 + max(oneMinusRoughness, float3(0.0, 0.0, 0.0)) * pow(1.0 - cosTheta, 5.0);
}

float3 mix(float3 x, float3 y, float a)
{
    return (1.0 - a) * x + a * y;
}

static const float3 albedo = 1.0;
static const float ao = 1.0;

float4 ps_main(out_vs input) : SV_TARGET
{
    float3 position = input.position.xyz;
    float metallic = input.position.w;
    float roughness = input.normal.w;

    float3 N = normalize(input.normal.xyz);
    float3 V = normalize(view_position - position);
    float3 R = reflect(-V, N);

    if (debug == 1)
        return float4(albedo, 1.0);
    else if (debug == 2)
        return float4(N, 1.0);
    else if (debug == 3)
        return float4(metallic, metallic, metallic, 1.0);
    else if (debug == 4)
        return float4(roughness, roughness, roughness, 1.0);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = mix(float3(0.04, 0.04, 0.04), albedo, metallic);

    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
#if 0
    for(int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        float3 delta = g_lights[i].position.xyz - position;
        float3 L = normalize(delta);
        float3 H = normalize(V + L);
        float distance = length(delta);
        float attenuation  = 1.0 / (distance * distance);
        float3 radiance = g_lights[i].color.rgb * attenuation;

        // Cook-Torracne BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        float3 nom = NDF * G * F;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float denom = 4.0 * NdotV * NdotV;
        float3 specular = nom / max(denom, 0.001); // prevent devide by 0

        // kS is equal to Fresnel
        float3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= (1.0 - metallic);

        // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
#endif

    // image based ambient lighting
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= (1.0 - metallic);
    float3 irradiance = irradianceMap.Sample(g_sampler, N).rgb;
    float3 diffuse = irradiance * albedo;

    // sample both pre-filtered map and BRDF lut and combine then together
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = specularMap.SampleLevel(g_samplerLod, R, roughness * MAX_REFLECTION_LOD).rgb;
    float reflectPower = max(dot(N, V), 0.0);

    float2 brdfUV = float2(reflectPower, 1.0 - roughness);
    float2 brdf = brdfLut.Sample(g_sampler, brdfUV).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    float3 ambient = (kD * diffuse + specular) * ao;

    float3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + 1.0);
    // gamma correction
    color = pow(color, 1.0 / 2.2);

    return float4(color, 1.0);
}

