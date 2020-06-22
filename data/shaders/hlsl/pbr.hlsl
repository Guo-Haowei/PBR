struct in_vs
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct out_vs
{
    float4 position : POSITION;
    float3 color : COLOR;
};

out_vs vs_main(in_vs input)
{
    out_vs output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    return output;
}

float4 ps_main(out_vs input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
