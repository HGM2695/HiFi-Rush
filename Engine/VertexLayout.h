#pragma once

#include "EngineCore.h"
#include <vector>

namespace gm
{
	enum class VertexElementSemantic
	{
		Position,
		Normal,
		TexCoord,
		Color,
	};

	enum class VertexElementFormat
	{
		Float2,
		Float3,
		Float4,
	};

	struct VertexElementDesc
	{
		VertexElementSemantic	semantic = VertexElementSemantic::Position;
		uint32					semanticIndex = 0;
		VertexElementFormat		format = VertexElementFormat::Float3;
		uint32					offset = 0;
	};

	struct VertexLayoutDesc
	{
		std::vector<VertexElementDesc> elements;
	};
}
