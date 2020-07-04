struct in_vs
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct out_vs
{
    float4 sv_position : SV_POSITION;
    float3 position : POSITION; // pack metallic in w component
    float3 normal : NORMAL; // pack roughess in w component
    float2 uv : TEXCOORD;
    float3x3 TBN : TBN;
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
    output.uv = input.uv;
    output.sv_position = mul(projection, mul(view, world_position));

    float3 T = normalize(mul(transform, float4(input.tangent, 0.0)).xyz);
    float3 B = normalize(mul(transform, float4(input.bitangent, 0.0)).xyz);
    float3 N = normalize(mul(transform, float4(input.normal, 0.0)).xyz);

    // output.TBN = float3x3(T.x, T.y, T.z, B.x, B.y, B.z, N.x, N.y, N.z);
    output.TBN = float3x3(T.x, B.x, N.x, T.y, B.y, N.y, T.z, B.z, N.z);
    return output;
}
