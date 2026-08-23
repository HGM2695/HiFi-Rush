cbuffer SkySphereCameraConstants : register(b0)
{
	row_major matrix view;
	row_major matrix projection;
	float3 cameraPosition;
	float cameraPadding;
};

struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float3 tangent : TANGENT;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 worldNormal : NORMAL;
	float3 viewDirection : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	const float3 worldPosition = input.position + cameraPosition;
	const float4 viewPosition = mul(float4(worldPosition, 1.f), view);
	output.position = mul(viewPosition, projection);
	output.texcoord = input.texcoord;
	output.worldNormal = input.normal;
	output.viewDirection = input.position;
	return output;
}
