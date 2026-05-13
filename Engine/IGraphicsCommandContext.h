#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

namespace gm
{
	class PipelineState;
	class Shader;
	class Mesh;
	class Texture;

	class IGraphicsCommandContext
	{
	public:
		virtual ~IGraphicsCommandContext() = default;

		virtual void		SetPipelineState(const PipelineState& desc) = 0;

		virtual void		SetPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void		SetVertexShader(const Shader& shader) = 0;
		virtual void		SetPixelShader(const Shader& shader) = 0;
		virtual void		SetMesh(Mesh& mesh) = 0;
		virtual void		SetTexture(uint32 slot, const Texture& texture) = 0;
		virtual void		DrawIndexed(uint32 indexCount) = 0;
	};
}
