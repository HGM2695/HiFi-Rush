#include "FogCommon.hlsli"
#include "GBufferCommon.hlsli"

cbuffer UVBarGraphConstants : register(b2)
{
    float4 barHeights;
    float4 barColor;
    float lineInterval;
    float lineThickness;
    float opacity;
    float emissiveIntensity;
};

float4 main(MeshPixelInput input) : SV_TARGET
{
    const float2 textureCoordinate = saturate(input.texcoord);
    const uint barIndex = min((uint)(textureCoordinate.x * 4.f), 3u);
    clip(textureCoordinate.y - (1.f - barHeights[barIndex]));

    const float linePosition = fmod(textureCoordinate.y, lineInterval);
    clip(lineInterval - lineThickness - linePosition);

    float3 color = lerp(1.f.xxx, barColor.rgb, textureCoordinate.y);
    color += color * emissiveIntensity;
    color = ApplyDepthFog(color, input.viewDepth);
    return float4(color, opacity);
}
