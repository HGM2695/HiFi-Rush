#include "ShadowCasterCommon.hlsli"

cbuffer ShadowViewConstants : register(b1)
{
    row_major matrix lightViewProjection;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float4 worldRow0 : TEXCOORD1;
    float4 worldRow1 : TEXCOORD2;
    float4 worldRow2 : TEXCOORD3;
    float4 worldRow3 : TEXCOORD4;
};

ShadowVertexOutput main(VSInput input)
{
    ShadowVertexOutput output;
    const row_major matrix world = float4x4(input.worldRow0, input.worldRow1, input.worldRow2, input.worldRow3);
    const float4 worldPosition = mul(float4(input.position, 1.f), world);
    output.position = mul(worldPosition, lightViewProjection);
    output.texcoord = input.texcoord;
    return output;
}
