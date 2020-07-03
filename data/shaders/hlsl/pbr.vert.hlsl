struct in_vs
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION; // pack metallic in w component
    float4 normal : NORMAL; // pack roughess in w component
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

out_vs vs_main(in_vs input, uint id : SV_InstanceID)
{
    out_vs output;
    float spacing = 2.5;
    int count = 5;
    int ix = id % count;
    int iy = id / count;
    output.normal.w = clamp(float(ix) / float(count), 0.05, 0.95); // roughness
    output.position.w = clamp(float(iy) / float(count), 0.05, 0.95); // metallic
    float x = spacing * (float(ix - count / 2));
    float y = spacing * (float(iy - count / 2));
    float3 offset = float3(x, y, 0.0);
    float4 world_position = float4(input.position + offset, 1.0);
    output.position.xyz = world_position.xyz;
    output.normal.xyz = input.normal;
    output.uv = input.uv;

    output.sv_position = mul(projection, mul(view, world_position));
    return output;
}
