#include "GBufferCommon.hlsli"

Texture2D baseColorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D ambientOcclusionTexture : register(t4);
Texture2D emissiveTexture : register(t5);

SamplerState baseColorSampler : register(s0);
SamplerState normalSampler : register(s1);
SamplerState ambientOcclusionSampler : register(s4);
SamplerState emissiveSampler : register(s5);

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

static const uint NormalTextureBit = 1 << 1;
static const uint AmbientOcclusionTextureBit = 1 << 4;
static const uint EmissiveTextureBit = 1 << 5;

GBufferOutput main(MeshPixelInput input)
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

    float3 worldNormal = normalize(input.worldNormal);
    if ((textureFlags & NormalTextureBit) != 0)
    {
        const float3 tangentSpaceNormal = normalize(normalTexture.Sample(normalSampler, textureCoordinate).xyz * 2.f - 1.f);
        const float3 worldTangent = normalize(input.worldTangent - worldNormal * dot(input.worldTangent, worldNormal));
        const float3 worldBitangent = normalize(cross(worldNormal, worldTangent));
        const float3x3 tangentToWorld = float3x3(worldTangent, worldBitangent, worldNormal);
        worldNormal = normalize(mul(tangentSpaceNormal, tangentToWorld));
    }

    float ambientOcclusion = 1.f;
    if ((textureFlags & AmbientOcclusionTextureBit) != 0)
        ambientOcclusion = ambientOcclusionTexture.Sample(ambientOcclusionSampler, textureCoordinate).r;

    float3 emissiveSource = baseColor.rgb;
    if ((textureFlags & EmissiveTextureBit) != 0)
        emissiveSource = emissiveTexture.Sample(emissiveSampler, textureCoordinate).rgb;
    float3 emissive = emissiveSource * emissiveColor.rgb * emissiveIntensity;

    const uint materialFlags = PackGBufferMaterialFlags(shadingModel, outlineMode != 0);
    GBufferOutput output;
    output.baseColor = baseColor;
    output.worldNormal = float4(worldNormal, 0.f);
    output.materialData = float4(ambientOcclusion, 0.f, 0.f, EncodeGBufferMaterialFlags(materialFlags));
    output.emissive = float4(emissive, 0.f);
    return output;
}
