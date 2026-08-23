Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    return sceneTexture.Sample(sceneSampler, input.texcoord);
}
