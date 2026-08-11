#pragma once

#include "Types.h"
#include <vector>

// 정점에 속하는 원소의 형식 그리고 정점 그 자체의 형식을 나타내는 구조체.
namespace gm
{
	enum class VertexElementSemantic
	{
		Position,
		Normal,
		TexCoord,
		Color,
		Tangent,
		BlendIndex,
		BlendWeight,

		Count
	};

	enum class VertexInputClassification
	{
		PerVertex,
		PerInstance,

		Count
	};

	enum class VertexElementFormat
	{
		Float2,
		Float3,
		Float4,
		UInt4,

		Count
	};

	// Vertex Shader 입력 엘리먼트 하나를 설명
	struct VertexElementDesc
	{
		VertexElementSemantic		semantic = VertexElementSemantic::Position;
		uint32						semanticIndex = 0;
		VertexElementFormat			format = VertexElementFormat::Float3;
		uint32						offset = 0;

		// 데이터를 가져올 Input Assembler의 입력 슬롯
		uint32						inputSlot = 0;
		uint32						instanceDataStepRate = 0;
		VertexInputClassification	inputClassification = VertexInputClassification::PerVertex;
	};

	// Vertex Shader에 전달할 입력 엘리먼트의 전체 구성
	struct VertexLayoutDesc
	{
		std::vector<VertexElementDesc> elements;
	};
}
