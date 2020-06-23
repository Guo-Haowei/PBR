struct in_vs
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct out_vs
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer PerObjectBuffer : register(b0)
{
    float4x4 model;
};

cbuffer PerFrameBuffer : register(b1)
{
    float4x4 view;
    float4x4 projection;
};

out_vs vs_main(in_vs input)
{
    out_vs output;
    float4 world_position = float4(input.position, 1.0);
    world_position = mul(view, world_position);
    world_position = mul(projection, world_position);
    output.position = world_position;
    output.color = input.color;
    return output;
}

float4 ps_main(out_vs input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
