#include "GBufferMaterialData.hlsli"

Texture2D debugTexture : register(t0);
SamplerState debugSampler : register(s0);

cbuffer RenderTargetDebugConstants : register(b0)
{
    row_major matrix inverseProjection;
    uint debugView;
    float valueScale;
    float2 padding;
};

static const uint BaseColorView = 1;
static const uint WorldNormalView = 2;
static const uint AmbientOcclusionView = 3;
static const uint ScreenSpaceAmbientOcclusionView = 4;
static const uint ScreenSpaceOutlineView = 5;
static const uint MaterialFlagsView = 6;
static const uint EmissiveView = 7;
static const uint SceneDepthView = 8;
static const uint BloomContributionView = 9;

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float VisualizeDepth(float2 texcoord, float depth)
{
    float2 ndcXY = float2(texcoord.x * 2.f - 1.f, 1.f - texcoord.y * 2.f);
    float4 viewPosition = mul(float4(ndcXY, depth, 1.f), inverseProjection);
    float4 farPosition = mul(float4(0.f, 0.f, 1.f, 1.f), inverseProjection);
    float viewDepth = abs(viewPosition.z / viewPosition.w);
    float farDepth = abs(farPosition.z / farPosition.w);
    return pow(saturate(viewDepth / max(farDepth, 0.0001f)), 0.25f);
}

float3 VisualizeMaterialFlags(float encodedFlags)
{
    uint materialFlags = DecodeGBufferMaterialFlags(encodedFlags);
    uint shadingModel = materialFlags & GBufferShadingModelMask;
    float outlineEnabled = (materialFlags & GBufferOutlineEnabledBit) != 0 ? 1.f : 0.f;
    float toonLit = shadingModel == GBufferToonLitShadingModel ? 1.f : 0.f;
    float lit = shadingModel == GBufferLitShadingModel ? 1.f : 0.f;
    float unlit = shadingModel == GBufferUnlitShadingModel ? 1.f : 0.f;
    return saturate(float3(outlineEnabled + lit, toonLit, unlit));
}

float4 main(PSInput input) : SV_TARGET
{
    float4 value = debugTexture.Sample(debugSampler, input.texcoord);
    if (debugView == BaseColorView)
        return float4(value.rgb, 1.f);
    if (debugView == WorldNormalView)
        return float4(value.xyz * 0.5f + 0.5f, 1.f);
    if (debugView == AmbientOcclusionView)
        return float4(value.rrr, 1.f);
	if (debugView == ScreenSpaceAmbientOcclusionView)
		return float4(value.rrr, 1.f);
	if (debugView == ScreenSpaceOutlineView)
		return float4(value.rrr, 1.f);
    if (debugView == MaterialFlagsView)
        return float4(VisualizeMaterialFlags(value.a), 1.f);
    if (debugView == EmissiveView)
        return float4(value.rgb / (1.f + value.rgb), 1.f);
    if (debugView == SceneDepthView)
        return float4(VisualizeDepth(input.texcoord, value.r).xxx, 1.f);
    if (debugView == BloomContributionView)
    {
        const float3 bloomContribution = value.rgb * valueScale;
        return float4(bloomContribution / (1.f + bloomContribution), 1.f);
    }
    return float4(value.rgb / (1.f + value.rgb), 1.f);
}
