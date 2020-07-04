struct in_vs
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION; // pack metallic in w component
    float3 normal : NORMAL; // pack roughess in w component
    float2 uv : TEXCOORD;
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

out_vs vs_main(in_vs input)
{
    out_vs output;
    float4 world_position = mul(transform, float4(input.position, 1.0));
    output.position.xyz = world_position.xyz;
    output.normal = mul(transform, float4(input.normal, 0.0)).xyz;
    output.uv = input.uv;
    output.sv_position = mul(projection, mul(view, world_position));

    return output;
}
