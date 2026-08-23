Texture2D sceneColorTexture : register(t0);
SamplerState sceneColorSampler : register(s0);

cbuffer FXAAConstants : register(b0)
{
    float2 texelSize;
    float edgeThreshold;
    float minimumEdgeThreshold;
    float subpixelBlending;
    float3 padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float CalcLuminance(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float4 main(PSInput input) : SV_TARGET
{
    const float3 centerColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord).rgb;
    const float3 northWestColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord + texelSize * float2(-1.f, -1.f)).rgb;
    const float3 northEastColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord + texelSize * float2(1.f, -1.f)).rgb;
    const float3 southWestColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord + texelSize * float2(-1.f, 1.f)).rgb;
    const float3 southEastColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord + texelSize * float2(1.f, 1.f)).rgb;
    const float centerLuminance = CalcLuminance(centerColor);
    const float northWestLuminance = CalcLuminance(northWestColor);
    const float northEastLuminance = CalcLuminance(northEastColor);
    const float southWestLuminance = CalcLuminance(southWestColor);
    const float southEastLuminance = CalcLuminance(southEastColor);
    const float minimumLuminance = min(centerLuminance, min(min(northWestLuminance, northEastLuminance), min(southWestLuminance, southEastLuminance)));
    const float maximumLuminance = max(centerLuminance, max(max(northWestLuminance, northEastLuminance), max(southWestLuminance, southEastLuminance)));
    const float luminanceRange = maximumLuminance - minimumLuminance;
    if (luminanceRange < max(minimumEdgeThreshold, maximumLuminance * edgeThreshold))
        return float4(centerColor, 1.f);

    float2 edgeDirection;
    edgeDirection.x = -((northWestLuminance + northEastLuminance) - (southWestLuminance + southEastLuminance));
    edgeDirection.y = (northWestLuminance + southWestLuminance) - (northEastLuminance + southEastLuminance);
    const float directionReduction = max((northWestLuminance + northEastLuminance + southWestLuminance + southEastLuminance) * 0.03125f, 0.0078125f);
    const float inverseMinimumDirection = 1.f / (min(abs(edgeDirection.x), abs(edgeDirection.y)) + directionReduction);
    edgeDirection = clamp(edgeDirection * inverseMinimumDirection, -8.f, 8.f) * texelSize;

    const float3 innerColor = 0.5f * (sceneColorTexture.Sample(sceneColorSampler, input.texcoord + edgeDirection * -0.1666667f).rgb + sceneColorTexture.Sample(sceneColorSampler, input.texcoord + edgeDirection * 0.1666667f).rgb);
    const float3 outerColor = innerColor * 0.5f + 0.25f * (sceneColorTexture.Sample(sceneColorSampler, input.texcoord + edgeDirection * -0.5f).rgb + sceneColorTexture.Sample(sceneColorSampler, input.texcoord + edgeDirection * 0.5f).rgb);
    const float outerLuminance = CalcLuminance(outerColor);
    const float3 antialiasedColor = outerLuminance < minimumLuminance || outerLuminance > maximumLuminance ? innerColor : outerColor;
    return float4(lerp(centerColor, antialiasedColor, subpixelBlending), 1.f);
}
