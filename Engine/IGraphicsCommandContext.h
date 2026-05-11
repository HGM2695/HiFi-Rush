#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

namespace gm
{
	class Shader;
	class Mesh;

	class IGraphicsCommandContext
	{
	public:
		virtual ~IGraphicsCommandContext() = default;

		virtual void		SetPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void		SetVertexShader(Shader& shader) = 0;
		virtual void		SetPixelShader(Shader& shader) = 0;
		virtual void		SetMesh(Mesh& mesh) = 0;
		virtual void		DrawIndexed(uint32 indexCount) = 0;
	};
}
