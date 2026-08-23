Texture2D sceneColorTexture : register(t0);
Texture2D bloomTexture : register(t1);
SamplerState linearSampler : register(s0);

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
    const float4 sceneColor = sceneColorTexture.Sample(linearSampler, input.texcoord);
    const float3 bloomColor = bloomTexture.Sample(linearSampler, input.texcoord).rgb;
    return float4(sceneColor.rgb + bloomColor * intensity, sceneColor.a);
}
