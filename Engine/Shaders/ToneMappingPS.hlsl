Texture2D sceneColorTexture : register(t0);
SamplerState sceneColorSampler : register(s0);

cbuffer ToneMappingConstants : register(b0)
{
    float exposure;
    float3 padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float3 FitACES(float3 color)
{
    const float3 numerator = color * (color + 0.0245786f) - 0.000090537f;
    const float3 denominator = color * (0.983729f * color + 0.4329510f) + 0.238081f;
    return numerator / denominator;
}

float3 ApplyACES(float3 color)
{
    const float3x3 inputTransform = float3x3(
        0.59719f, 0.35458f, 0.04823f,
        0.07600f, 0.90834f, 0.01566f,
        0.02840f, 0.13383f, 0.83777f);
    const float3x3 outputTransform = float3x3(
         1.60475f, -0.53108f, -0.07367f,
        -0.10208f,  1.10813f, -0.00605f,
        -0.00327f, -0.07276f,  1.07602f);
    color = mul(inputTransform, color);
    color = FitACES(color);
    color = mul(outputTransform, color);
    return saturate(color);
}

float4 main(PSInput input) : SV_TARGET
{
    const float3 hdrColor = sceneColorTexture.Sample(sceneColorSampler, input.texcoord).rgb * exp2(exposure);
    return float4(ApplyACES(hdrColor), 1.f);
}
