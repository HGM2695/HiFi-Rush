#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

namespace gm
{
	class PipelineState;
	class Shader;
	class Mesh;
	class Texture;
	class Sampler;
	class Material;
	class ConstantBuffer;

	class IGraphicsCommandContext
	{
	public:
		virtual ~IGraphicsCommandContext() = default;

		virtual void		SetPipelineState(const PipelineState& state) = 0;

		virtual void		SetPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void		SetVertexShader(const Shader& shader) = 0;
		virtual void		SetPixelShader(const Shader& shader) = 0;
		virtual void		SetMesh(const Mesh& mesh) = 0;
		virtual void		SetMaterial(const Material& material) = 0;

		virtual void		SetTexture(uint32 slot, const Texture* texture) = 0;
		virtual void		SetSampler(uint32 slot, const Sampler* sampler) = 0;

		virtual void		SetConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer) = 0;
		virtual void		UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size) = 0;

		virtual void		DrawIndexed(uint32 indexCount) = 0;
	};
}
