#ifndef GBUFFER_COMMON_HLSLI
#define GBUFFER_COMMON_HLSLI

#include "GBufferMaterialData.hlsli"

struct MeshPixelInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
    float viewDepth : TEXCOORD3;
};

struct GBufferOutput
{
    float4 baseColor : SV_TARGET0;
    float4 worldNormal : SV_TARGET1;
    float4 materialData : SV_TARGET2;
    float4 emissive : SV_TARGET3;
};

float3 TransformWorldNormal(float3 normal, float3x3 transform)
{
    float3 row0 = transform[0];
    float3 row1 = transform[1];
    float3 row2 = transform[2];
    float3x3 normalTransform = float3x3(cross(row1, row2), cross(row2, row0), cross(row0, row1));
    float determinantSign = dot(row0, cross(row1, row2)) < 0.f ? -1.f : 1.f;
    return normalize(mul(normal, normalTransform) * determinantSign);
}

float3 TransformWorldTangent(float3 tangent, float3x3 transform)
{
    return normalize(mul(tangent, transform));
}

#endif
