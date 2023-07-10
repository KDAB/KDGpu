struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer Entity : register ( b0 )
{
    float4x4 model;
};

PSInput main(float3 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = mul(model, float4(position, 1.0f));
    result.color = color;

    return result;
}
