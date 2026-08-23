#pragma once

#include "EngineCore.h"
#include "GraphicsTypes.h"
#include <vector>

namespace gm
{
	class Shader;
	class Mesh;
	class Texture;
	class Material;
	class ConstantBuffer;
	class InstanceBuffer;

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
		virtual void		UnbindPixelShader() = 0;

		virtual void		BindMesh(const Mesh& mesh) = 0;
		virtual void		BindInstanceBuffer(const InstanceBuffer& buffer) = 0;

		virtual void		BindShaderTexture(uint32 slot, const Texture* texture) = 0;
		virtual void		BindSampler(uint32 slot, const SamplerDesc* samplerDesc) = 0;
		virtual void		UnbindShaderTextures(uint32 startSlot, uint32 count) = 0;

		virtual void		BindBackBuffer() = 0;
		virtual void		BindRenderTarget(const Texture* renderTexture, const Texture* depthTexture) = 0;
		virtual void		BindRenderTargets(const std::vector<const Texture*>& renderTextures, const Texture* depthTexture) = 0;
		virtual void		BindDepthStencilSlice(const Texture& depthTexture, uint32 arraySlice) = 0;
		virtual void		ClearBackBuffer(const Color& color, float depth = 1.f, uint8 stencil = 0) = 0;
		virtual void		ClearRenderTarget(Texture& renderTexture, const Color& color) = 0;
		virtual void		ClearDepthStencil(Texture& depthTexture, float depth = 1.f, uint8 stencil = 0) = 0;
		virtual void		ClearDepthStencilSlice(Texture& depthTexture, uint32 arraySlice, float depth = 1.f, uint8 stencil = 0) = 0;
		virtual void		SetViewport(const Viewport& viewport) = 0;

		virtual void		BindConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer) = 0;
		virtual void		UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size) = 0;
		virtual bool		UpdateInstanceBuffer(InstanceBuffer& buffer, const void* data, uint32 instanceCount) = 0;

		virtual void		DrawIndexed(uint32 indexCount) = 0;
		virtual void		DrawIndexed(uint32 indexCount, uint32 startIndexLocation, int32 baseVertexLocation) = 0;
		virtual void		DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 startIndexLocation, int32 baseVertexLocation, uint32 startInstanceLocation = 0) = 0;
	};
}
