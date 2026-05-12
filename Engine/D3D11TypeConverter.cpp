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
		default:
			GM_ASSERT_RETURN_VAL(false, DXGI_FORMAT_UNKNOWN, "지원하지 않는 VertexElementFormat입니다.");
		}

		static_assert(static_cast<uint32>(VertexElementFormat::Count) == 3, "VertexElementFormat을 추가했다면, D3D11로의 변환도 고려");
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
		default:
			GM_ASSERT_RETURN_VAL(false, "", "지원하지 않는 VertexElementSemant,.ic입니다.");
		}

		static_assert(static_cast<uint32>(VertexElementSemantic::Count) == 4, "VertexElementSemantic을 추가했다면, D3D11로의 변환도 고려");
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
				0,
				element.offset,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			});
		}

		return inputElements;
	}
}
