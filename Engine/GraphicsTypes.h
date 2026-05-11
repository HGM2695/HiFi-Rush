// GraphicsTypes.h
#pragma once

namespace gm
{
    enum class GraphicsAPI
    {
        D3D11,
    };

    enum class PrimitiveTopology
    {
        TriangleList,
        LineList,
        PointList,
    };

    enum class FillMode
    {
        Solid,
        Wireframe,
    };

    enum class CullMode
    {
        None,
        Front,
        Back,
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
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcAlpha,
        InvSrcAlpha,
    };

    enum class BlendOp
    {
        Add,
        Subtract,
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
}
