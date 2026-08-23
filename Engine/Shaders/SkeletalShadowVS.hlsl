#include "ShadowCasterCommon.hlsli"

static const uint MaxSkinningBoneCount = 512;

cbuffer ObjectConstants : register(b0)
{
    row_major matrix world;
};

cbuffer ShadowViewConstants : register(b1)
{
    row_major matrix lightViewProjection;
};

cbuffer BonePaletteConstants : register(b2)
{
    row_major matrix boneMatrices[MaxSkinningBoneCount];
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    uint4 blendIndex : BLENDINDEX0;
    float4 blendWeight : BLENDWEIGHT0;
};

ShadowVertexOutput main(VSInput input)
{
    const row_major matrix skinMatrix =
        boneMatrices[input.blendIndex.x] * input.blendWeight.x +
        boneMatrices[input.blendIndex.y] * input.blendWeight.y +
        boneMatrices[input.blendIndex.z] * input.blendWeight.z +
        boneMatrices[input.blendIndex.w] * input.blendWeight.w;

    ShadowVertexOutput output;
    const float4 skinnedPosition = mul(float4(input.position, 1.f), skinMatrix);
    const float4 worldPosition = mul(skinnedPosition, world);
    output.position = mul(worldPosition, lightViewProjection);
    output.texcoord = input.texcoord;
    return output;
}
