#pragma once

#include "Types.h"
#include <memory>

namespace gm
{
    enum class GraphicsAPI
    {
        D3D11,
        Count
    };

    enum class ShaderStage
    {
        Vertex,
        Pixel,
        //Geometry,
        //Compute,
        //Hull,
        //Domain
    };

    enum class PrimitiveTopology
    {
        TriangleList,
        LineList,
        PointList,
        Count
    };

    enum class FillMode
    {
        Solid,
        Wireframe,
        Count
    };

    enum class CullMode
    {
        None,
        Front,
        Back,
        Count
    };

    enum class CompareFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
        Count
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcAlpha,
        InvSrcAlpha,
        Count
    };

    enum class BlendOp
    {
        Add,
        Subtract,
        Count
    };

    enum class TextureFilter
    {
        Point,
        Linear,
        Anisotropic,
        Count
    };

    enum class TextureAddressMode
    {
        Wrap,
        Clamp,
        Border,
        Count
    };

    struct DepthStencilDesc
    {
        bool        depthEnable = true;
        bool        depthWriteEnable = true;
        CompareFunc depthFunc = CompareFunc::LessEqual;
        bool        stencilEnable = false;
    };

    struct RasterizerDesc
    {
        FillMode    fillMode = FillMode::Solid;
        CullMode    cullMode = CullMode::Back;
        bool        frontCounterClockwise = false;
        bool        depthClipEnable = true;
    };

    struct BlendDesc
    {
        bool        blendEnable = false;
        BlendFactor srcBlend = BlendFactor::SrcAlpha;
        BlendFactor destBlend = BlendFactor::InvSrcAlpha;
        BlendOp     blendOp = BlendOp::Add;
    };

    enum class MaterialSlot
    {
        BaseColor,
        Normal,
        Roughness,
        Metallic,
        AmbientOcclusion,
        Emissive,

        Custom0,
        Custom1,
        Custom2,

        Count
    };

    inline constexpr uint32 ToMaterialSlotIndex(MaterialSlot slot) { return static_cast<uint32>(slot); }
    inline constexpr MaterialSlot ToMaterialSlot(uint32 index) { return static_cast<MaterialSlot>(index); }
    inline constexpr uint32 MaterialSlotCount = ToMaterialSlotIndex(MaterialSlot::Count);
}
