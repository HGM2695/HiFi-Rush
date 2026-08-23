#include "DeferredCommon.hlsli"

Texture2D sceneDepthTexture : register(t0);
Texture2D worldNormalTexture : register(t1);
Texture2D noiseTexture : register(t2);
SamplerState pointClampSampler : register(s0);
SamplerState linearWrapSampler : register(s1);

static const uint MaxSSAOSampleCount = 16;

cbuffer SSAOConstants : register(b0)
{
	row_major matrix inverseViewProjection;
	row_major matrix projection;
	row_major matrix viewMatrix;
	float4 sampleKernel[MaxSSAOSampleCount];
	float2 noiseScale;
	float radius;
	float fadeStart;
	float fadeEnd;
	float surfaceEpsilon;
	float strength;
	float padding;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
	const float sceneDepth = sceneDepthTexture.SampleLevel(pointClampSampler, input.texcoord, 0.f).r;
	if (sceneDepth >= 0.999999f)
		return 1.f;

	const float3 worldPosition = ScreenUVToWorldPosition(input.texcoord, sceneDepth, inverseViewProjection);
	const float3 viewPosition = mul(float4(worldPosition, 1.f), viewMatrix).xyz;
	const float3 worldNormal = normalize(worldNormalTexture.SampleLevel(pointClampSampler, input.texcoord, 0.f).xyz);
	const float3 viewNormal = normalize(mul(worldNormal, (float3x3)viewMatrix));
	const float3 randomDirection = normalize(noiseTexture.SampleLevel(linearWrapSampler, input.texcoord * noiseScale, 0.f).xyz * 2.f - 1.f);

	float occlusion = 0.f;
	for (uint sampleIndex = 0; sampleIndex < MaxSSAOSampleCount; ++sampleIndex)
	{
		const float3 reflectedDirection = reflect(sampleKernel[sampleIndex].xyz, randomDirection);
		const float hemisphereSign = dot(reflectedDirection, viewNormal) < 0.f ? -1.f : 1.f;
		const float3 sampleViewPosition = viewPosition + reflectedDirection * hemisphereSign * radius;
		const float4 sampleClipPosition = mul(float4(sampleViewPosition, 1.f), projection);
		if (sampleClipPosition.w <= 0.f)
			continue;
		const float3 sampleNDC = sampleClipPosition.xyz / sampleClipPosition.w;
		const float2 sampleUV = float2(sampleNDC.x * 0.5f + 0.5f, 0.5f - sampleNDC.y * 0.5f);
		if (any(sampleUV < 0.f) || any(sampleUV > 1.f))
			continue;
		const float sampledDepth = sceneDepthTexture.SampleLevel(pointClampSampler, sampleUV, 0.f).r;
		if (sampledDepth >= 0.999999f)
			continue;
		const float3 sampledWorldPosition = ScreenUVToWorldPosition(sampleUV, sampledDepth, inverseViewProjection);
		const float3 sampledViewPosition = mul(float4(sampledWorldPosition, 1.f), viewMatrix).xyz;
		const float depthDifference = viewPosition.z - sampledViewPosition.z;
		if (depthDifference <= surfaceEpsilon)
			continue;
		const float fade = saturate((fadeEnd - depthDifference) / (fadeEnd - fadeStart));
		const float normalAlignment = max(dot(viewNormal, normalize(sampledViewPosition - viewPosition)), 0.f);
		const float rangeWeight = 0.5f * smoothstep(0.f, 1.f, radius / max(abs(depthDifference), 0.0001f));
		occlusion += normalAlignment * fade * rangeWeight;
	}

	const float ambientOcclusion = saturate(1.f - occlusion / float(MaxSSAOSampleCount) * strength);
	return float4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1.f);
}
