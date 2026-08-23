#include "GBufferCommon.hlsli"

cbuffer ObjectConstants : register(b0)
{
    row_major matrix world;
};

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
};

MeshPixelInput main(VSInput input)
{
    MeshPixelInput output;

    float4 worldPosition = mul(float4(input.position, 1.f), world);
    float4 viewPosition = mul(worldPosition, view);
    output.position = mul(viewPosition, projection);
    output.texcoord = input.texcoord;
    output.worldNormal = TransformWorldNormal(input.normal, (float3x3)world);
    output.worldTangent = TransformWorldTangent(input.tangent, (float3x3)world);
    output.viewDepth = viewPosition.z;
    return output;
}
