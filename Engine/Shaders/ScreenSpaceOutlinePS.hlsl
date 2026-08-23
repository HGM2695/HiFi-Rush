#include "DeferredCommon.hlsli"
#include "GBufferMaterialData.hlsli"

Texture2D sceneColorTexture : register(t0);
Texture2D sceneDepthTexture : register(t1);
Texture2D worldNormalTexture : register(t2);
Texture2D materialDataTexture : register(t3);
SamplerState pointClampSampler : register(s0);

cbuffer ScreenSpaceOutlineConstants : register(b0)
{
	row_major matrix inverseViewProjection;
	row_major matrix viewMatrix;
	float2 texelSize;
	float depthThreshold;
	float normalThreshold;
	float4 outlineColor;
	float thickness;
	uint debugOutput;
	float2 padding;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

bool IsOutlineEnabled(float2 texcoord)
{
	const float encodedFlags = materialDataTexture.SampleLevel(pointClampSampler, texcoord, 0.f).a;
	return (DecodeGBufferMaterialFlags(encodedFlags) & GBufferOutlineEnabledBit) != 0;
}

float3 GetWorldPosition(float2 texcoord, float depth)
{
	return ScreenUVToWorldPosition(texcoord, depth, inverseViewProjection);
}

float CalcOutline(float2 texcoord, float3 centerWorldPosition, float centerViewDepth, float3 centerNormal)
{
	static const float2 directions[8] =
	{
		float2(-1.f, 0.f), float2(1.f, 0.f), float2(0.f, -1.f), float2(0.f, 1.f),
		float2(-1.f, -1.f), float2(1.f, -1.f), float2(-1.f, 1.f), float2(1.f, 1.f)
	};

	float maximumDepthDifference = 0.f;
	float maximumNormalDifference = 0.f;
	for (uint directionIndex = 0; directionIndex < 8; ++directionIndex)
	{
		const float2 sampleTexcoord = texcoord + directions[directionIndex] * texelSize * thickness;
		const float sampleDepth = sceneDepthTexture.SampleLevel(pointClampSampler, sampleTexcoord, 0.f).r;
		if (sampleDepth >= 0.999999f)
		{
			maximumDepthDifference = 1.f;
			maximumNormalDifference = 1.f;
			continue;
		}
		if (IsOutlineEnabled(sampleTexcoord) == false)
			continue;

		const float3 sampleWorldPosition = GetWorldPosition(sampleTexcoord, sampleDepth);
		const float distanceFromCenterPlane = abs(dot(sampleWorldPosition - centerWorldPosition, centerNormal));
		const float relativeDepthDifference = distanceFromCenterPlane / max(abs(centerViewDepth), 1.f);
		maximumDepthDifference = max(maximumDepthDifference, relativeDepthDifference);
		const float3 sampleNormal = normalize(worldNormalTexture.SampleLevel(pointClampSampler, sampleTexcoord, 0.f).xyz);
		maximumNormalDifference = max(maximumNormalDifference, 1.f - saturate(dot(centerNormal, sampleNormal)));
	}

	const float depthEdge = smoothstep(depthThreshold, depthThreshold * 2.f, maximumDepthDifference);
	const float normalEdge = smoothstep(normalThreshold, normalThreshold + 0.1f, maximumNormalDifference);
	return max(depthEdge, normalEdge);
}

float4 main(PSInput input) : SV_TARGET
{
	const float4 sceneColor = sceneColorTexture.SampleLevel(pointClampSampler, input.texcoord, 0.f);
	const float centerDepth = sceneDepthTexture.SampleLevel(pointClampSampler, input.texcoord, 0.f).r;
	if (centerDepth >= 0.999999f || IsOutlineEnabled(input.texcoord) == false)
		return debugOutput != 0 ? float4(0.f, 0.f, 0.f, 1.f) : sceneColor;

	const float3 centerNormal = normalize(worldNormalTexture.SampleLevel(pointClampSampler, input.texcoord, 0.f).xyz);
	const float3 centerWorldPosition = GetWorldPosition(input.texcoord, centerDepth);
	const float centerViewDepth = mul(float4(centerWorldPosition, 1.f), viewMatrix).z;
	const float outline = CalcOutline(input.texcoord, centerWorldPosition, centerViewDepth, centerNormal) * outlineColor.a;
	if (debugOutput != 0)
		return float4(outline, outline, outline, 1.f);
	return float4(lerp(sceneColor.rgb, outlineColor.rgb, outline), sceneColor.a);
}
