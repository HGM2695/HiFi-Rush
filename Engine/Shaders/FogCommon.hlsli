#ifndef FOG_COMMON_HLSLI
#define FOG_COMMON_HLSLI

cbuffer DepthFogConstants : register(b1)
{
    float4 fogColor;
    float fogStartDistance;
    float fogEndDistance;
    float fogDensity;
    uint fogEnabled;
};

float CalcDepthFogAmount(float viewDepth)
{
    const float fogRange = max(fogEndDistance - fogStartDistance, 0.0001f);
    const float fogDistanceRatio = saturate((viewDepth - fogStartDistance) / fogRange);
    const float fogAmount = 1.f - exp(-max(fogDensity, 0.f) * fogDistanceRatio);
    return fogEnabled != 0 ? fogAmount : 0.f;
}

float3 ApplyDepthFog(float3 color, float viewDepth)
{
    return lerp(color, fogColor.rgb, CalcDepthFogAmount(viewDepth));
}

#endif
