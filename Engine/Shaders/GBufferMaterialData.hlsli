#ifndef GBUFFER_MATERIAL_DATA_HLSLI
#define GBUFFER_MATERIAL_DATA_HLSLI

static const uint GBufferShadingModelMask = (1 << 0) | (1 << 1);
static const uint GBufferOutlineEnabledBit = 1 << 2;
static const uint GBufferUnlitShadingModel = 0;
static const uint GBufferToonLitShadingModel = 1;
static const uint GBufferLitShadingModel = 2;

uint PackGBufferMaterialFlags(uint shadingModel, bool outlineEnabled)
{
    uint flags = shadingModel & GBufferShadingModelMask;
    
    if (outlineEnabled)
        flags |= GBufferOutlineEnabledBit;
    
    return flags;
}

float EncodeGBufferMaterialFlags(uint materialFlags)
{
    return materialFlags / 255.f;
}

uint DecodeGBufferMaterialFlags(float encodedFlags)
{
    return (uint)round(saturate(encodedFlags) * 255.f);
}

#endif
