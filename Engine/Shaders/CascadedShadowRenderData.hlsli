static const uint MaxShadowCascadeCount = 4;

cbuffer CascadedShadowConstants : register(b2)
{
    row_major matrix    cascadeViewProjections[MaxShadowCascadeCount];
    float4              cascadeSplitDistances;
    uint                cascadeCount;
    uint                shadowPCFRadius;
    float               shadowDepthBias;
    float               shadowNormalBias;
    float               shadowMapTexelSize;
    float               shadowStrength;
    float2              cascadedShadowPadding0;
    float3              shadowColor;
    float               cascadedShadowPadding1;
};
