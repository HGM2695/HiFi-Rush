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

float4 main(PSInput input) : SV_TARGET
{
    const float2 texel = sourceTexelSize;
    const float3 center = sourceTexture.Sample(sourceSampler, input.texcoord).rgb;
    const float3 corners = sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(-1.f, -1.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(1.f, -1.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(-1.f, 1.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(1.f, 1.f)).rgb;
    const float3 edges = sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(0.f, -1.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(-1.f, 0.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(1.f, 0.f)).rgb + sourceTexture.Sample(sourceSampler, input.texcoord + texel * float2(0.f, 1.f)).rgb;
    return float4((center * 4.f + edges * 2.f + corners) / 16.f, 0.f);
}
