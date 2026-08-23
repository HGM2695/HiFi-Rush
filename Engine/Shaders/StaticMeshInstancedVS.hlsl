#include "GBufferCommon.hlsli"

cbuffer CameraConstants : register(b1)
{
    row_major matrix view;
    row_major matrix projection;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
    float3 tangent : TANGENT;
    float4 worldRow0 : TEXCOORD1;
    float4 worldRow1 : TEXCOORD2;
    float4 worldRow2 : TEXCOORD3;
    float4 worldRow3 : TEXCOORD4;
};

MeshPixelInput main(VSInput input)
{
    MeshPixelInput output;

    float4x4 world = float4x4(input.worldRow0, input.worldRow1, input.worldRow2, input.worldRow3);
    float4 worldPosition = mul(float4(input.position, 1.f), world);
    float4 viewPosition = mul(worldPosition, view);
    output.position = mul(viewPosition, projection);
    output.texcoord = input.texcoord;
    output.worldNormal = TransformWorldNormal(input.normal, (float3x3)world);
    output.worldTangent = TransformWorldTangent(input.tangent, (float3x3)world);
    output.viewDepth = viewPosition.z;
    return output;
}
