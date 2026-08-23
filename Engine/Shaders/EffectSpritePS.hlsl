#include "EffectCommon.hlsli"

Texture2D       baseColorTexture : register(t0);
Texture2D       dissolveTexture : register(t6);
SamplerState    baseColorSampler : register(s0);
SamplerState    dissolveSampler : register(s6);

cbuffer SpriteConstants : register(b0)
{
    float2  uvOffset;
    float2  uvScale;
    float4  blendColor;
    float   blendRatio;
    float   spriteOpacity;
    float   fillRatio;
	uint    fillMode;
	float   radialStartAngle;
	float   radialSweepAngle;
	float2  radialCenter;
};

cbuffer EffectMaterialConstants : register(b2)
{
    float4  emissiveColor;
    float   dissolveThreshold;
    float   emissiveIntensity;
    uint    dissolveEnabled;
    float   effectPadding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

void ApplyFill(float2 texcoord)
{
    if (fillMode == 0)
    {
        clip(saturate(fillRatio) - texcoord.x);
    }
    else if (fillMode == 1)
    {
        static const float TwoPi = 6.28318530718f;
		float2 direction = texcoord - radialCenter;
        float angle = atan2(direction.y, direction.x);
        if (angle < 0.f)
            angle += TwoPi;

        float deltaAngle = angle - radialStartAngle;
        if (deltaAngle < 0.f)
            deltaAngle += TwoPi;

        clip(saturate(fillRatio) * radialSweepAngle - deltaAngle);
    }
    else
    {
        clip(texcoord.x - (1.f - saturate(fillRatio)));
    }
}

float4 main(PSInput input) : SV_TARGET
{
    ApplyFill(input.texcoord);
    float2 uv = uvOffset + input.texcoord * uvScale;
    float4 color = baseColorTexture.Sample(baseColorSampler, uv);
    clip(color.a - 0.001f);
	ApplyEffectDissolve(dissolveEnabled, dissolveTexture.Sample(dissolveSampler, input.texcoord).r, dissolveThreshold);
    color.rgb = lerp(color.rgb, blendColor.rgb, saturate(blendRatio));
    color.rgb = ApplyEffectEmissive(color.rgb, emissiveColor.rgb, emissiveIntensity);
    color.a *= saturate(spriteOpacity);
    return color;
}
