#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"

namespace gm
{
	class Shader;
	class Mesh;
	class Texture;
	class Material;
	class ConstantBuffer;

	class IGraphicsCommandContext
	{
	public:
		virtual ~IGraphicsCommandContext() = default;

		virtual void		BindMaterial(const Material& material) = 0;

		virtual void		BindPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void		BindRasterizerState(const RasterizerDesc& desc) = 0;
		virtual void		BindDepthStencilState(const DepthStencilDesc& desc) = 0;
		virtual void		BindBlendState(const BlendDesc& desc) = 0;

		virtual void		BindVertexShader(const Shader& shader) = 0;
		virtual void		BindPixelShader(const Shader& shader) = 0;

		virtual void		BindMesh(const Mesh& mesh) = 0;

		virtual void		BindTexture(uint32 slot, const Texture* texture) = 0;
		virtual void		BindSampler(uint32 slot, const SamplerDesc* samplerDesc) = 0;

		virtual void		BindConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer) = 0;
		virtual void		UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size) = 0;

		virtual void		DrawIndexed(uint32 indexCount) = 0;
	};
}
