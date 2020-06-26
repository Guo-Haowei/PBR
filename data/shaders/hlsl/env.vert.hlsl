struct in_vs
{
    float3 position : POSITION;
};

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION;
};

cbuffer PerFrameBuffer : register(b1)
{
    float4x4 view;
    float4x4 projection;
};

out_vs vs_main(in_vs input)
{
    out_vs output;
    output.position = input.position;
    // output.sv_position = float4(output.position, 1.0);
    // float scale = 20.0f;
    float4 world_position = float4(input.position, 1.0);
    output.sv_position = mul(projection, mul(view, world_position));
    return output;
}
