#pragma once

#include "MathTypes.h"
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

	struct VertexMesh
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
		Vector3 tangent;

		static VertexLayoutDesc GetLayout()
		{
			return {
				std::vector<VertexElementDesc>
				{
					{ VertexElementSemantic::Position, 0, VertexElementFormat::Float3, offsetof(VertexMesh, position) },
					{ VertexElementSemantic::Normal, 0, VertexElementFormat::Float3, offsetof(VertexMesh, normal) },
					{ VertexElementSemantic::TexCoord, 0, VertexElementFormat::Float2, offsetof(VertexMesh, texcoord) },
					{ VertexElementSemantic::Tangent, 0, VertexElementFormat::Float3, offsetof(VertexMesh, tangent) }
				}
			};
		}
	};

	struct VertexAnimationMesh
	{
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
		Vector3 tangent;

		uint32	blendIndex;
		Vector4	blendWeight;

		static VertexLayoutDesc GetLayout()
		{
			return {
				std::vector<VertexElementDesc>
				{
					{ VertexElementSemantic::Position, 0, VertexElementFormat::Float3, offsetof(VertexAnimationMesh, position) },
					{ VertexElementSemantic::Normal, 0, VertexElementFormat::Float3, offsetof(VertexAnimationMesh, normal) },
					{ VertexElementSemantic::TexCoord, 0, VertexElementFormat::Float2, offsetof(VertexAnimationMesh, texcoord) },
					{ VertexElementSemantic::Tangent, 0, VertexElementFormat::Float3, offsetof(VertexAnimationMesh, tangent) },
					{ VertexElementSemantic::BlendIndex, 0, VertexElementFormat::UInt4, offsetof(VertexAnimationMesh, blendIndex) },
					{ VertexElementSemantic::BlendWeight, 0, VertexElementFormat::Float4, offsetof(VertexAnimationMesh, blendWeight) }
				}
			};
		}
	};
}
