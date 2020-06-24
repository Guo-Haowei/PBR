#include <metal_stdlib>
using namespace metal;

struct VS_OUT
{
    float4 position [[position]];
    float3 color;
};

vertex VS_OUT vs_main(constant float4* in[[buffer(0)]], uint vid[[vertex_id]])
{
    VS_OUT out;
    out.position = in[vid];
    out.color = in[vid].xyz + float3(0.5);
    return out;
}

fragment float4 fs_main(VS_OUT in[[stage_in]])
{
    return float4(in.color, 1);
}