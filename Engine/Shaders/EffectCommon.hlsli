#ifndef EFFECT_COMMON_HLSLI
#define EFFECT_COMMON_HLSLI

void ApplyEffectDissolve(uint dissolveEnabled, float dissolveValue, float dissolveThreshold)
{
	if (dissolveEnabled != 0)
		clip(dissolveValue - dissolveThreshold);
}

float3 ApplyEffectEmissive(float3 color, float3 emissiveColor, float emissiveIntensity)
{
    return color + color * emissiveColor * max(emissiveIntensity, 0.f);
}

#endif
