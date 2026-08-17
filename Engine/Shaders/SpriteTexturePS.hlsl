Texture2D       g_texture : register(t0);
SamplerState    g_sampler : register(s0);

cbuffer sprite : register(b0)
{
    float2 uvOffset;
    float2 uvScale;
    float4 blendColor;
    float blendRatio;
    float opacity;
    float2 padding;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{   
    float2 uv = uvOffset + input.texcoord * uvScale;
    float4 color = g_texture.Sample(g_sampler, uv);
    clip(color.a - 0.001f);

    color.rgb = lerp(color.rgb, blendColor.rgb, saturate(blendRatio));
    color.a *= saturate(opacity);

    return color;
}
