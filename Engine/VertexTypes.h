#pragma once

#include "EngineCore.h"
#include "VertexLayout.h"
#include <cstddef>

namespace gm
{
	struct VertexPosCol
	{
		Vector3 position;
		Color	color;

		static VertexLayoutDesc GetLayout()
		{
			return {
				std::vector<VertexElementDesc>
				{
					{ VertexElementSemantic::Position, 0, VertexElementFormat::Float3, offsetof(VertexPosCol, position) },
					{ VertexElementSemantic::Color, 0, VertexElementFormat::Float4, offsetof(VertexPosCol, color) },
				}
			};
		}
	};

	struct VertexPosTex
	{
		Vector3 position;
		Vector2	texcoord;

		static VertexLayoutDesc GetLayout()
		{
			return {
				std::vector<VertexElementDesc>
				{
					{ VertexElementSemantic::Position, 0, VertexElementFormat::Float3, offsetof(VertexPosTex, position) },
					{ VertexElementSemantic::TexCoord, 0, VertexElementFormat::Float2, offsetof(VertexPosTex, texcoord) },
				}
			};
		}
	};

	struct VertexPosNormTex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;

		static VertexLayoutDesc GetLayout()
		{
			return {
				std::vector<VertexElementDesc>
				{
					{ VertexElementSemantic::Position, 0, VertexElementFormat::Float3, offsetof(VertexPosNormTex, position) },
					{ VertexElementSemantic::Normal, 0, VertexElementFormat::Float3, offsetof(VertexPosNormTex, normal) },
					{ VertexElementSemantic::TexCoord, 0, VertexElementFormat::Float2, offsetof(VertexPosNormTex, texcoord) },
				}
			};
		}
	};
}
