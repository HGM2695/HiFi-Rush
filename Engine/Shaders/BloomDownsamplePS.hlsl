Texture2D sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

cbuffer BloomConstants : register(b0)
{
    float2 sourceTexelSize;
    float threshold;
    float softKnee;
    float intensity;
    uint applyThreshold;
    float2 padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float3 FilterDownsample(float2 texcoord)
{
    const float2 texel = sourceTexelSize;
    const float3 center = sourceTexture.Sample(sourceSampler, texcoord).rgb;
    const float3 farCorners = sourceTexture.Sample(sourceSampler, texcoord + texel * float2(-2.f, -2.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(2.f, -2.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(-2.f, 2.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(2.f, 2.f)).rgb;
    const float3 farEdges = sourceTexture.Sample(sourceSampler, texcoord + texel * float2(0.f, -2.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(-2.f, 0.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(2.f, 0.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(0.f, 2.f)).rgb;
    const float3 nearCorners = sourceTexture.Sample(sourceSampler, texcoord + texel * float2(-1.f, -1.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(1.f, -1.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(-1.f, 1.f)).rgb + sourceTexture.Sample(sourceSampler, texcoord + texel * float2(1.f, 1.f)).rgb;
    return center * 0.125f + farCorners * 0.03125f + farEdges * 0.0625f + nearCorners * 0.125f;
}

float3 ApplyThreshold(float3 color)
{
    const float brightness = max(color.r, max(color.g, color.b));
    const float knee = max(threshold * softKnee, 0.0001f);
    float softContribution = clamp(brightness - threshold + knee, 0.f, knee * 2.f);
    softContribution = softContribution * softContribution / (knee * 4.f);
    const float contribution = max(softContribution, brightness - threshold) / max(brightness, 0.0001f);
    return color * contribution;
}

float4 main(PSInput input) : SV_TARGET
{
    float3 color = FilterDownsample(input.texcoord);
    if (applyThreshold != 0)
        color = ApplyThreshold(color);
    return float4(color, 1.f);
}
