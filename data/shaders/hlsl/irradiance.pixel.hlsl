#define PI 3.14159265359

TextureCube envTexture : register(t0);
SamplerState envSampler : register(s1);

static const float sampleStep = 0.025;

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION;
};

float4 ps_main(out_vs input) : SV_TARGET
{
    float3 N = normalize(input.position);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = cross(up, N);
    up = cross(N, right);

    float3 irradiance = float3(0.0, 0.0, 0.0);
    float samples = 0.0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleStep)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleStep)
        {
            float xdir = sin(theta) * cos(phi);
            float ydir = sin(theta) * sin(phi);
            float zdir = cos(theta);
            float3 sampleVec = xdir * right + ydir * up + zdir * N;
            float3 color = envTexture.SampleLevel(envSampler, sampleVec, 0.0).rgb;
            irradiance += color * cos(theta) * sin(theta);
            samples += 1.0;
        }
    }

    irradiance = PI * irradiance * (1.0 / samples);
    return float4(irradiance, 1.0);
}
