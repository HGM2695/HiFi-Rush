Texture2D sourceTexture : register(t0);
SamplerState linearClampSampler : register(s0);

cbuffer SSAOBlurConstants : register(b0)
{
	float2 texelSize;
	float2 padding;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
	const float2 uv = input.texcoord;
	const float2 texel = texelSize;
	float ambientOcclusion = sourceTexture.Sample(linearClampSampler, uv).r * 0.125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(-2.f, 2.f) * texel).r * 0.03125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(2.f, 2.f) * texel).r * 0.03125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(-2.f, -2.f) * texel).r * 0.03125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(2.f, -2.f) * texel).r * 0.03125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(0.f, 2.f) * texel).r * 0.0625f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(-2.f, 0.f) * texel).r * 0.0625f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(2.f, 0.f) * texel).r * 0.0625f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(0.f, -2.f) * texel).r * 0.0625f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(-1.f, 1.f) * texel).r * 0.125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(1.f, 1.f) * texel).r * 0.125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(-1.f, -1.f) * texel).r * 0.125f;
	ambientOcclusion += sourceTexture.Sample(linearClampSampler, uv + float2(1.f, -1.f) * texel).r * 0.125f;
	return ambientOcclusion;
}
