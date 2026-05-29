Texture2D       g_texture : register(t0);
SamplerState    g_sampler : register(s0);

cbuffer sprite : register(b0)
{
    float leftNorm;
    float topNorm;
    float widthNorm;
    float heightNorm;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{   
    float2 uv = float2(leftNorm + input.texcoord.x * widthNorm, topNorm + input.texcoord.y * heightNorm);
    float4 color = g_texture.Sample(g_sampler, uv);
    clip(color.a - 0.001f);

    return color;
}