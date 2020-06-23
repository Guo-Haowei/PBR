struct in_vs
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

cbuffer PerObjectBuffer : register(b0)
{
    float4x4 transform;
};

cbuffer PerFrameBuffer : register(b1)
{
    float4x4 view;
    float4x4 projection;
};

out_vs vs_main(in_vs input, uint id : SV_InstanceID)
{
    float scale = 3.0;
    float x = scale * (float(id % 4) - 1.5);
    float y = scale * (float(id / 4) - 1.5);
    float3 offset = float3(x, y, 0.0);

    out_vs output;
    // float4 world_position = mul(transform, float4(input.position, 1.0));
    float4 world_position = float4(input.position + offset, 1.0);
    output.position = world_position.xyz;
    world_position = mul(view, world_position);
    world_position = mul(projection, world_position);
    output.sv_position = world_position;
    output.uv = input.uv;
    output.normal = input.normal;
    return output;
}

float4 ps_main(out_vs input) : SV_TARGET
{
    const float3 light_position = float3(0.0, 10.0, -10.0);
    float3 N = normalize(input.normal);
    float3 L = normalize(light_position - input.position);
    float d = max(dot(N, L), 0.0);
    float3 color = float3(0.9, 0.9, 0.9);

    return float4(d * color, 1.0);
}
