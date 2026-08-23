#include "ShadowCasterCommon.hlsli"

cbuffer ObjectConstants : register(b0)
{
    row_major matrix world;
};

cbuffer ShadowViewConstants : register(b1)
{
    row_major matrix lightViewProjection;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

ShadowVertexOutput main(VSInput input)
{
    ShadowVertexOutput output;
    const float4 worldPosition = mul(float4(input.position, 1.f), world);
    output.position = mul(worldPosition, lightViewProjection);
    output.texcoord = input.texcoord;
    return output;
}
