#include "D3D11TypeConverter.h"

namespace gm
{
	D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::TriangleList:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::LineList:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::PointList:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED, "지원하지 않는 PrimitiveTopology입니다.");
		}

		static_assert(static_cast<uint32>(PrimitiveTopology::Count) == 3, "PrimitiveTopology를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_FILL_MODE ToD3D11FillMode(FillMode fillMode)
	{
		switch (fillMode)
		{
		case FillMode::Solid:
			return D3D11_FILL_SOLID;
		case FillMode::Wireframe:
			return D3D11_FILL_WIREFRAME;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_FILL_SOLID, "지원하지 않는 FillMode입니다.");
		}

		static_assert(static_cast<uint32>(FillMode::Count) == 2, "FillMode를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_CULL_MODE ToD3D11CullMode(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CullMode::None:
			return D3D11_CULL_NONE;
		case CullMode::Front:
			return D3D11_CULL_FRONT;
		case CullMode::Back:
			return D3D11_CULL_BACK;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_CULL_BACK, "지원하지 않는 CullMode입니다.");
		}

		static_assert(static_cast<uint32>(CullMode::Count) == 3, "CullMode를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(CompareFunc compareFunc)
	{
		switch (compareFunc)
		{
		case CompareFunc::Never:
			return D3D11_COMPARISON_NEVER;
		case CompareFunc::Less:
			return D3D11_COMPARISON_LESS;
		case CompareFunc::Equal:
			return D3D11_COMPARISON_EQUAL;
		case CompareFunc::LessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;
		case CompareFunc::Greater:
			return D3D11_COMPARISON_GREATER;
		case CompareFunc::NotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;
		case CompareFunc::GreaterEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case CompareFunc::Always:
			return D3D11_COMPARISON_ALWAYS;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_COMPARISON_LESS_EQUAL, "지원하지 않는 CompareFunc입니다.");
		}

		static_assert(static_cast<uint32>(CompareFunc::Count) == 8, "CompareFunc를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_BLEND ToD3D11Blend(BlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case BlendFactor::Zero:
			return D3D11_BLEND_ZERO;
		case BlendFactor::One:
			return D3D11_BLEND_ONE;
		case BlendFactor::SrcAlpha:
			return D3D11_BLEND_SRC_ALPHA;
		case BlendFactor::InvSrcAlpha:
			return D3D11_BLEND_INV_SRC_ALPHA;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_BLEND_ONE, "지원하지 않는 BlendFactor입니다.");
		}

		static_assert(static_cast<uint32>(BlendFactor::Count) == 4, "BlendFactor를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_BLEND_OP ToD3D11BlendOp(BlendOp blendOp)
	{
		switch (blendOp)
		{
		case BlendOp::Add:
			return D3D11_BLEND_OP_ADD;
		case BlendOp::Subtract:
			return D3D11_BLEND_OP_SUBTRACT;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_BLEND_OP_ADD, "지원하지 않는 BlendOp입니다.");
		}

		static_assert(static_cast<uint32>(BlendOp::Count) == 2, "BlendOp를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_FILTER ToD3D11Filter(TextureFilter filter)
	{
		switch (filter)
		{
		case TextureFilter::Point:
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilter::Linear:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case TextureFilter::Anisotropic:
			return D3D11_FILTER_ANISOTROPIC;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_FILTER_MIN_MAG_MIP_LINEAR, "지원하지 않는 TextureFilter입니다.");
		}

		static_assert(static_cast<uint32>(TextureFilter::Count) == 3, "TextureFilter를 추가했다면, D3D11로의 변환도 고려");
	}

	D3D11_TEXTURE_ADDRESS_MODE ToD3D11TextureAddressMode(TextureAddressMode addressMode)
	{
		switch (addressMode)
		{
		case TextureAddressMode::Wrap:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureAddressMode::Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TextureAddressMode::Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		default:
			GM_ASSERT_RETURN_VAL(false, D3D11_TEXTURE_ADDRESS_CLAMP, "지원하지 않는 TextureAddressMode입니다.");
		}

		static_assert(static_cast<uint32>(TextureAddressMode::Count) == 3, "TextureAddressMode을 추가했다면, D3D11로의 변환도 고려");
	}

	DXGI_FORMAT ToD3D11TextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R8UNorm:
			return DXGI_FORMAT_R8_UNORM;
		case TextureFormat::R16Float:
			return DXGI_FORMAT_R16_FLOAT;
		case TextureFormat::R32Float:
			return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::R32UInt:
			return DXGI_FORMAT_R32_UINT;
		case TextureFormat::RGBA8UNorm:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA8UNormSRGB:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case TextureFormat::RGBA16Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::Depth24Stencil8:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case TextureFormat::Depth32Float:
			return DXGI_FORMAT_R32_TYPELESS;
		default:
			GM_ASSERT_RETURN_VAL(false, DXGI_FORMAT_UNKNOWN, "지원하지 않는 TextureFormat입니다.");
		}

		static_assert(static_cast<uint32>(TextureFormat::Count) == 10, "TextureFormat을 추가했다면, D3D11로의 변환도 고려");
	}

	DXGI_FORMAT ToD3D11SRVFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RGBA8UNormSRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case TextureFormat::Depth24Stencil8:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case TextureFormat::Depth32Float:
			return DXGI_FORMAT_R32_FLOAT;
		default:
			return ToD3D11TextureFormat(format);
		}
	}

	DXGI_FORMAT ToD3D11RTVFormat(TextureFormat format)
	{
		GM_ASSERT_RETURN_VAL(format != TextureFormat::Depth24Stencil8 && format != TextureFormat::Depth32Float, DXGI_FORMAT_UNKNOWN, "Depth TextureFormat은 Render Target으로 사용할 수 없습니다.");
		return format == TextureFormat::RGBA8UNormSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : ToD3D11TextureFormat(format);
	}

	DXGI_FORMAT ToD3D11DSVFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::Depth24Stencil8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::Depth32Float:
			return DXGI_FORMAT_D32_FLOAT;
		default:
			GM_ASSERT_RETURN_VAL(false, DXGI_FORMAT_UNKNOWN, "Depth Stencil View로 지원하지 않는 TextureFormat입니다.");
		}
	}

	uint32 ToD3D11BindFlags(TextureBindUsage usage)
	{
		uint32 bindFlags = 0;
		if (HasTextureBindUsage(usage, TextureBindUsage::ShaderResource))
			bindFlags |= D3D11_BIND_SHADER_RESOURCE;
		if (HasTextureBindUsage(usage, TextureBindUsage::RenderTarget))
			bindFlags |= D3D11_BIND_RENDER_TARGET;
		if (HasTextureBindUsage(usage, TextureBindUsage::DepthStencil))
			bindFlags |= D3D11_BIND_DEPTH_STENCIL;
		return bindFlags;
	}

	DXGI_FORMAT ToDXGIFormat(VertexElementFormat format)
	{
		switch (format)
		{
		case VertexElementFormat::Float2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case VertexElementFormat::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case VertexElementFormat::Float4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case VertexElementFormat::UInt4:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		default:
			GM_ASSERT_RETURN_VAL(false, DXGI_FORMAT_UNKNOWN, "지원하지 않는 VertexElementFormat입니다.");
		}

		static_assert(static_cast<uint32>(VertexElementFormat::Count) == 4, "VertexElementFormat을 추가했다면, D3D11로의 변환도 고려");
	}

	const char* ToD3D11SemanticName(VertexElementSemantic semantic)
	{
		switch (semantic)
		{
		case VertexElementSemantic::Position:
			return "POSITION";
		case VertexElementSemantic::Normal:
			return "NORMAL";
		case VertexElementSemantic::TexCoord:
			return "TEXCOORD";
		case VertexElementSemantic::Color:
			return "COLOR";
		case VertexElementSemantic::Tangent:
			return "TANGENT";
		case VertexElementSemantic::BlendIndex:
			return "BLENDINDEX";
		case VertexElementSemantic::BlendWeight:
			return "BLENDWEIGHT";
		default:
			GM_ASSERT_RETURN_VAL(false, "", "지원하지 않는 VertexElementSemant,.ic입니다.");
		}

		static_assert(static_cast<uint32>(VertexElementSemantic::Count) == 7, "VertexElementSemantic을 추가했다면, D3D11로의 변환도 고려");
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputElements(const VertexLayoutDesc& layoutDesc)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
		inputElements.reserve(layoutDesc.elements.size());

		for (const VertexElementDesc& element : layoutDesc.elements)
		{
			inputElements.push_back(
			{
				ToD3D11SemanticName(element.semantic),
				element.semanticIndex,
				ToDXGIFormat(element.format),
				element.inputSlot,
				element.offset,
				element.inputClassification == VertexInputClassification::PerInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA,
				element.instanceDataStepRate
			});
		}

		return inputElements;
	}
}
