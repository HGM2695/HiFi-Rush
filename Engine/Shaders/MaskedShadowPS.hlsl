#include "ShadowCasterCommon.hlsli"

Texture2D baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

cbuffer ShadowMaterialConstants : register(b0)
{
    float alphaCutoff;
    float3 padding;
    float2 textureUVOffset;
    float2 textureUVPadding;
};

void main(ShadowVertexOutput input)
{
    const float alpha = baseColorTexture.Sample(baseColorSampler, input.texcoord + textureUVOffset).a;
    clip(alpha - alphaCutoff);
}
