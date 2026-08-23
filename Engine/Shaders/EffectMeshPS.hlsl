#include "EffectCommon.hlsli"
#include "FogCommon.hlsli"
#include "GBufferCommon.hlsli"

Texture2D baseColorTexture : register(t0);
Texture2D dissolveTexture : register(t6);
SamplerState baseColorSampler : register(s0);
SamplerState dissolveSampler : register(s6);

cbuffer MaterialSurfaceConstants : register(b0)
{
    uint    shadingModel;
    uint    surfaceMode;
    uint    outlineMode;
    float   materialEmissiveIntensity;
    float   alphaCutoff;
    float3  surfacePadding;
    float4  materialEmissiveColor;
    uint    colorMode;
    uint    textureFlags;
    float   colorBlendRatio;
    float   colorPadding;
    float4  blendColor;
    float4  opacityLowColor;
    float4  opacityHighColor;
    float4  colorMultiplier;
    float2  textureUVOffset;
    float2  textureUVPadding;
};

cbuffer EffectMaterialConstants : register(b2)
{
    float4  effectEmissiveColor;
    float   dissolveThreshold;
    float   effectEmissiveIntensity;
    uint    dissolveEnabled;
    float   effectPadding;
};

float4 main(MeshPixelInput input) : SV_TARGET
{
    float2 textureCoordinate = input.texcoord + textureUVOffset;
    float4 color = baseColorTexture.Sample(baseColorSampler, textureCoordinate);
    clip(color.a - 0.001f);
    ApplyEffectDissolve(dissolveEnabled, dissolveTexture.Sample(dissolveSampler, textureCoordinate).r, dissolveThreshold);

    if (colorMode == 1)
        color.rgb = lerp(color.rgb, blendColor.rgb, saturate(colorBlendRatio));
    else if (colorMode == 2)
        color.rgb *= lerp(opacityLowColor.rgb, opacityHighColor.rgb, color.a);
    color.rgb *= colorMultiplier.rgb;
    color.a *= colorMultiplier.a;
    color.rgb += color.rgb * materialEmissiveColor.rgb * max(materialEmissiveIntensity, 0.f);
    color.rgb = ApplyEffectEmissive(color.rgb, effectEmissiveColor.rgb, effectEmissiveIntensity);
    color.rgb = ApplyDepthFog(color.rgb, input.viewDepth);
    return color;
}
