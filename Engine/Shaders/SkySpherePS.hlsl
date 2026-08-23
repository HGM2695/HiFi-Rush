#include "GBufferCommon.hlsli"

Texture2D baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 worldNormal : NORMAL;
	float3 viewDirection : TEXCOORD1;
};

GBufferOutput main(PSInput input)
{
	const float4 baseColor = baseColorTexture.Sample(baseColorSampler, input.texcoord + float2(0.f, 0.1f));
	const float3 reflectedLight = reflect(normalize(float3(-1.f, 3.f, 1.f)), normalize(input.worldNormal));
	const float specular = pow(max(dot(-reflectedLight, normalize(input.viewDirection)), 0.f), 100.f);
	const uint materialFlags = PackGBufferMaterialFlags(GBufferUnlitShadingModel, false);

	GBufferOutput output;
	output.baseColor = float4(baseColor.rgb + specular, baseColor.a);
	output.worldNormal = float4(normalize(input.worldNormal), 0.f);
	output.materialData = float4(1.f, 0.f, 0.f, EncodeGBufferMaterialFlags(materialFlags));
	output.emissive = float4(0.f, 0.f, 0.f, 0.f);
	return output;
}
