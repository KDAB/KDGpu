struct PSInput
{
    float4 color : COLOR;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = input.color;
    return output;
}
