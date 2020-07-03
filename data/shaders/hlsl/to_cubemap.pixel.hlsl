Texture2D envTexture : register(t0);

SamplerState envSampler : register(s0);

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION;
};

static const float2 invAtan = float2(0.1591, 0.3183);

float2 sampleSphericalMap(in float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    // uv.y = 1.0 - uv.y;
    return uv;
}

float4 ps_main(out_vs input) : SV_TARGET
{
    float2 uv = sampleSphericalMap(normalize(input.position));
    float3 color = envTexture.Sample(envSampler, uv).rgb;
    return float4(color, 1.0);
}
