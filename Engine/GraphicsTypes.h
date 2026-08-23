#pragma once

#include "Types.h"
#include "GMAssert.h"
#include <memory>

namespace gm
{
    enum class GraphicsAPI
    {
        D3D11,
        Count
    };

	struct Viewport
	{
		float x = 0.f;
		float y = 0.f;
		float width = 0.f;
		float height = 0.f;
		float minDepth = 0.f;
		float maxDepth = 1.f;
	};

    enum class ShaderStage
    {
        Vertex,
        Pixel,
        //Geometry,
        //Compute,
        //Hull,
        //Domain
        Count
    };

    inline constexpr uint32 ShaderStageCount = static_cast<uint32>(ShaderStage::Count);
    inline constexpr uint32 MaxConstantBufferSlots = 16;

    inline constexpr uint32 ToShaderStageIndex(ShaderStage stage)
    {
        const uint32 index = static_cast<uint32>(stage);
        GM_ASSERT_RETURN_VAL(index < ShaderStageCount, 0, "잘못된 쉐이더 스테이지 입력입니다.");

        return index;
    }

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

        bool operator==(const DepthStencilDesc& rhs) const
        {
            return depthEnable == rhs.depthEnable && depthWriteEnable == rhs.depthWriteEnable && depthFunc == rhs.depthFunc && stencilEnable == rhs.stencilEnable;
        }
    };

    struct RasterizerDesc
    {
        FillMode    fillMode = FillMode::Solid;
        CullMode    cullMode = CullMode::Back;
        bool        frontCounterClockwise = false;
        bool        depthClipEnable = true;

        bool operator==(const RasterizerDesc& rhs) const
        {
            return fillMode == rhs.fillMode && cullMode == rhs.cullMode && frontCounterClockwise == rhs.frontCounterClockwise && depthClipEnable == rhs.depthClipEnable;
        }
    };

    struct BlendDesc
    {
        bool        blendEnable = false;
        BlendFactor srcBlend = BlendFactor::SrcAlpha;
        BlendFactor destBlend = BlendFactor::InvSrcAlpha;
        BlendOp     blendOp = BlendOp::Add;

        bool operator==(const BlendDesc& rhs) const
        {
            return blendEnable == rhs.blendEnable && srcBlend == rhs.srcBlend && destBlend == rhs.destBlend && blendOp == rhs.blendOp;
        }
    };

    struct SamplerDesc
    {
        TextureFilter       filter = TextureFilter::Linear;
        TextureAddressMode  addressU = TextureAddressMode::Clamp;
        TextureAddressMode  addressV = TextureAddressMode::Clamp;
        TextureAddressMode  addressW = TextureAddressMode::Clamp;

        bool operator==(const SamplerDesc& rhs) const
        {
            return filter == rhs.filter && addressU == rhs.addressU && addressV == rhs.addressV && addressW == rhs.addressW;
        }
    };

    enum class TextureSlot
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

	enum class TextureColorSpace
	{
		Linear,
		SRGB,

		Count
	};

	enum class TextureFormat
	{
		Unknown,
		R8UNorm, // 단일 채널만 사용하는 경우
		R16Float, // 계산용 단일 채널
		R32Float,
		R32UInt, // Object ID 등
		RGBA8UNorm, // LDR
		RGBA8UNormSRGB, // sRGB 출력용
		RGBA16Float, // HDR
		Depth24Stencil8, // Depth, Stencil
		Depth32Float, // Shadow Depth

		Count
	};

	enum class TextureBindUsage : uint8
	{
		None = 0,
		ShaderResource = 1 << 0,
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2
	};

	inline constexpr TextureBindUsage operator|(TextureBindUsage lhs, TextureBindUsage rhs) { return static_cast<TextureBindUsage>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs)); }
	inline constexpr TextureBindUsage operator&(TextureBindUsage lhs, TextureBindUsage rhs) { return static_cast<TextureBindUsage>(static_cast<uint8>(lhs) & static_cast<uint8>(rhs)); }
	inline constexpr bool HasTextureBindUsage(TextureBindUsage usage, TextureBindUsage value) { return (usage & value) == value; }

    inline constexpr uint32             ToTexturelSlotIndex(TextureSlot slot) { return static_cast<uint32>(slot); }
    inline constexpr TextureSlot        ToTextureSlot(uint32 index) { return static_cast<TextureSlot>(index); }
    inline constexpr uint32             TextureSlotCount = ToTexturelSlotIndex(TextureSlot::Count);
    // BaseColor 슬롯이면 색으로 읽고, Normal 슬롯이면 수치 데이터로 읽습니다
	inline constexpr TextureColorSpace  GetDefaultTextureColorSpace(TextureSlot slot) { return slot == TextureSlot::BaseColor || slot == TextureSlot::Emissive ? TextureColorSpace::SRGB : TextureColorSpace::Linear; }
}
