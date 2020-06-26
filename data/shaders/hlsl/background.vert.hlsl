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
    float4x4 viewRotation = view;
    // remove transformation
    viewRotation[0][3] = 0.0;
    viewRotation[1][3] = 0.0;
    viewRotation[2][3] = 0.0;
    float4 world_position = float4(input.position, 1.0);
    output.sv_position = mul(projection, mul(viewRotation, world_position));
    output.sv_position.z = 0.99999 * output.sv_position.w;
    // output.sv_position.z = output.sv_position.w;
    return output;
}

