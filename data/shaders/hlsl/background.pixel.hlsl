TextureCube envTexture : register(t0);

SamplerState envSampler : register(s0);

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION;
};

float4 ps_main(out_vs input) : SV_TARGET
{
    float3 uvw = input.position;
    uvw.y = -uvw.y;
    float3 env_color = envTexture.Sample(envSampler, uvw).rgb;
    env_color = env_color / (env_color + float3(1.0, 1.0, 1.0));
    float ratio = 1.0 / 2.2;
    env_color = pow(env_color, float3(ratio, ratio, ratio));
    return float4(env_color, 1.0);
}

