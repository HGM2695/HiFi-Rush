#include "FogCommon.hlsli"

Texture2D sceneColorTexture : register(t0);
Texture2D sceneDepthTexture : register(t1);
SamplerState sceneColorSampler : register(s0);
SamplerState depthSampler : register(s1);

cbuffer DepthFogCameraConstants : register(b0)
{
    row_major matrix inverseProjection;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float GetLinearViewDepth(float2 texcoord, float depth)
{
    const float2 ndcPosition = float2(texcoord.x * 2.f - 1.f, 1.f - texcoord.y * 2.f);
    const float4 viewPosition = mul(float4(ndcPosition, depth, 1.f), inverseProjection);
    return viewPosition.z / viewPosition.w;
}

float4 main(PSInput input) : SV_TARGET
{
    const float4 sceneColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord);
    const float sceneDepth = sceneDepthTexture.Sample(depthSampler, input.texcoord).r;
    if (sceneDepth >= 0.999999f)
        return sceneColor;

    const float viewDepth = GetLinearViewDepth(input.texcoord, sceneDepth);
    return float4(ApplyDepthFog(sceneColor.rgb, viewDepth), sceneColor.a);
}
