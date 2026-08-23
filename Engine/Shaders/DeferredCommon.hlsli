#ifndef DEFERRED_COMMON_HLSLI
#define DEFERRED_COMMON_HLSLI

float3 ScreenUVToWorldPosition(float2 texcoord, float depth, float4x4 inverseViewProjection)
{
    float2 ndcXY = float2(texcoord.x * 2.f - 1.f, 1.f - texcoord.y * 2.f);
    float4 worldPosition = mul(float4(ndcXY, depth, 1.f), inverseViewProjection);
    return worldPosition.xyz / worldPosition.w;
}

#endif
