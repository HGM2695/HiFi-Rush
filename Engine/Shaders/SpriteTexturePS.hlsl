Texture2D       g_texture : register(t0);
SamplerState    g_sampler : register(s0);

cbuffer sprite : register(b0)
{
    float2 uvOffset;
    float2 uvScale;
    float4 blendColor;
    float blendRatio;
    float opacity;
    float fillRatio;
    uint fillMode;
    float radialStartAngle;
    float radialSweepAngle;
    float2 padding;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    if (fillMode == 0)
    {
        clip(saturate(fillRatio) - input.texcoord.x);
    }
    else if (fillMode == 1)
    {
        static const float TwoPi = 6.28318530718f;
        float2 direction = input.texcoord - float2(0.5f, 0.5f);
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
        clip(input.texcoord.x - (1.f - saturate(fillRatio)));
    }

    float2 uv = uvOffset + input.texcoord * uvScale;
    float4 color = g_texture.Sample(g_sampler, uv);
    clip(color.a - 0.001f);

    color.rgb = lerp(color.rgb, blendColor.rgb, saturate(blendRatio));
    color.a *= saturate(opacity);

    return color;
}
