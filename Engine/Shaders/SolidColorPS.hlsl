struct PSInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

cbuffer ColorConstants : register(b0)
{
	float4 color;
};

float4 main(PSInput input) : SV_TARGET
{
	return color;
}
