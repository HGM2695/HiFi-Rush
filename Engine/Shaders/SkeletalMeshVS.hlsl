#include "GBufferCommon.hlsli"

static const uint MaxSkinningBoneCount = 512;

cbuffer ObjectConstants : register(b0)
{
    row_major matrix world;
};

cbuffer CameraConstants : register(b1)
{
    row_major matrix view;
    row_major matrix projection;
}

cbuffer BonePaletteConstants : register(b2)
{
    row_major matrix boneMatrices[MaxSkinningBoneCount];
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
    float3 tangent : TANGENT;
    uint4 blendIndex : BLENDINDEX0;
    float4 blendWeight : BLENDWEIGHT0;
};

MeshPixelInput main(VSInput input)
{
    MeshPixelInput output;

    row_major matrix skinMatrix =
        boneMatrices[input.blendIndex.x] * input.blendWeight.x +
        boneMatrices[input.blendIndex.y] * input.blendWeight.y +
        boneMatrices[input.blendIndex.z] * input.blendWeight.z +
        boneMatrices[input.blendIndex.w] * input.blendWeight.w;

    float4 skinnedPosition = mul(float4(input.position, 1.f), skinMatrix);
    float4 worldPosition = mul(skinnedPosition, world);
    float4 viewPosition = mul(worldPosition, view);
    float3 skinnedNormal = TransformWorldNormal(input.normal, (float3x3)skinMatrix);
    float3 skinnedTangent = TransformWorldTangent(input.tangent, (float3x3)skinMatrix);

    output.position = mul(viewPosition, projection);
    output.texcoord = input.texcoord;
    output.worldNormal = TransformWorldNormal(skinnedNormal, (float3x3)world);
    output.worldTangent = TransformWorldTangent(skinnedTangent, (float3x3)world);
    output.viewDepth = viewPosition.z;
    return output;
}
