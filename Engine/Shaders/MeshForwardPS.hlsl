#include "FogCommon.hlsli"
#include "GBufferCommon.hlsli"

Texture2D       baseColorTexture : register(t0);
Texture2D       emissiveTexture : register(t5);
SamplerState    baseColorSampler : register(s0);
SamplerState    emissiveSampler : register(s5);

cbuffer MaterialSurfaceConstants : register(b0)
{
    uint shadingModel;
    uint surfaceMode;
    uint outlineMode;
    float emissiveIntensity;
    float alphaCutoff;
    float3 surfacePadding;
    float4 emissiveColor;
    uint colorMode;
    uint textureFlags;
    float colorBlendRatio;
    float colorPadding;
    float4 blendColor;
    float4 opacityLowColor;
    float4 opacityHighColor;
    float4 colorMultiplier;
    float2 textureUVOffset;
    float2 textureUVPadding;
};

static const uint EmissiveTextureBit = 1 << 5;

float4 main(MeshPixelInput input) : SV_TARGET
{
    const float2 textureCoordinate = input.texcoord + textureUVOffset;
    float4 baseColor = baseColorTexture.Sample(baseColorSampler, textureCoordinate);
    if (surfaceMode == 1)
        clip(baseColor.a - alphaCutoff);

    if (colorMode == 1)
        baseColor.rgb = lerp(baseColor.rgb, blendColor.rgb, saturate(colorBlendRatio));
    else if (colorMode == 2)
        baseColor.rgb *= lerp(opacityLowColor.rgb, opacityHighColor.rgb, baseColor.a);
	baseColor.rgb *= colorMultiplier.rgb;
	baseColor.a *= colorMultiplier.a;

    float3 emissiveSource = baseColor.rgb;
    if ((textureFlags & EmissiveTextureBit) != 0)
        emissiveSource = emissiveTexture.Sample(emissiveSampler, textureCoordinate).rgb;
    baseColor.rgb += emissiveSource * emissiveColor.rgb * emissiveIntensity;
    baseColor.rgb = ApplyDepthFog(baseColor.rgb, input.viewDepth);
    return baseColor;
}
