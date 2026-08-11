#pragma once

#include "MathTypes.h"
#include "VertexLayout.h"
#include <array>
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

		std::array<uint32, 4>	blendIndex{};
		Vector4					blendWeight;

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

	struct VertexMeshInstanced
	{
		static VertexLayoutDesc GetLayout()
		{
			VertexLayoutDesc layout = VertexMesh::GetLayout();
			for (uint32 rowIndex = 0; rowIndex < 4; ++rowIndex)
			{
				VertexElementDesc element{};
				element.semantic = VertexElementSemantic::TexCoord;
				element.semanticIndex = rowIndex + 1;
				element.format = VertexElementFormat::Float4;
				element.offset = rowIndex * sizeof(Vector4);
				element.inputSlot = 1;
				element.instanceDataStepRate = 1;
				element.inputClassification = VertexInputClassification::PerInstance;
				layout.elements.push_back(element);
			}

			return layout;
		}
	};
}
