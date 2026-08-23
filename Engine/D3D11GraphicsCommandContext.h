#pragma once

#include "D3D11RenderStateManager.h"
#include "IGraphicsCommandContext.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace gm
{
	class D3D11GraphicsDevice;

	class D3D11GraphicsCommandContext : public IGraphicsCommandContext
	{
	public:
		explicit D3D11GraphicsCommandContext(D3D11GraphicsDevice& graphicsDevice);

		virtual void		BindMaterial(const Material& material) override;

		virtual void		BindPrimitiveTopology(PrimitiveTopology topology) override;
		virtual void		BindRasterizerState(const RasterizerDesc& desc) override;
		virtual void		BindDepthStencilState(const DepthStencilDesc& desc) override;
		virtual void		BindBlendState(const BlendDesc& desc) override;

		virtual void		BindVertexShader(const Shader& shader) override;
		virtual void		BindPixelShader(const Shader& shader) override;
		virtual void		UnbindPixelShader() override;

		virtual void		BindMesh(const Mesh& mesh) override;
		virtual void		BindInstanceBuffer(const InstanceBuffer& buffer) override;

		virtual void		BindShaderTexture(uint32 slot, const Texture* texture) override;
		virtual void		BindSampler(uint32 slot, const SamplerDesc* samplerDesc) override;
		virtual void		UnbindShaderTextures(uint32 startSlot, uint32 count) override;

		virtual void		BindBackBuffer() override;
		virtual void		BindRenderTarget(const Texture* renderTexture, const Texture* depthTexture) override;
		virtual void		BindRenderTargets(const std::vector<const Texture*>& renderTextures, const Texture* depthTexture) override;
		virtual void		BindDepthStencilSlice(const Texture& depthTexture, uint32 arraySlice) override;
		virtual void		ClearBackBuffer(const Color& color, float depth = 1.f, uint8 stencil = 0) override;
		virtual void		ClearRenderTarget(Texture& renderTexture, const Color& color) override;
		virtual void		ClearDepthStencil(Texture& depthTexture, float depth = 1.f, uint8 stencil = 0) override;
		virtual void		ClearDepthStencilSlice(Texture& depthTexture, uint32 arraySlice, float depth = 1.f, uint8 stencil = 0) override;
		virtual void		SetViewport(const Viewport& viewport) override;

		virtual void		BindConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer) override;
		virtual void		UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size) override;
		virtual bool		UpdateInstanceBuffer(InstanceBuffer& buffer, const void* data, uint32 instanceCount) override;

		virtual void		DrawIndexed(uint32 indexCount) override;
		virtual void		DrawIndexed(uint32 indexCount, uint32 startIndexLocation, int32 baseVertexLocation) override;
		virtual void		DrawIndexedInstanced(uint32 indexCount, uint32 instanceCount, uint32 startIndexLocation, int32 baseVertexLocation, uint32 startInstanceLocation = 0) override;

	private:
		void				BindNativeVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout);
		void				BindNativePixelShader(ID3D11PixelShader* pixelShader);
		void				BindNativeVertexBuffer(uint32 slot, ID3D11Buffer* vertexBuffer, uint32 stride);
		void				BindNativeIndexBuffer(ID3D11Buffer* indexBuffer);
		void				BindNativeTexture(uint32 slot, ID3D11ShaderResourceView* shaderResourceView);
		void				BindNativeSampler(uint32 slot, ID3D11SamplerState* samplerState);
		void				BindNativeRasterizerState(ID3D11RasterizerState* rasterizerState);
		void				BindNativeDepthStencilState(ID3D11DepthStencilState* depthStencilState);
		void				BindNativeBlendState(ID3D11BlendState* blendState);

		D3D11GraphicsDevice&	_graphicsDevice;
		ID3D11Device*			_device = nullptr;
		ID3D11DeviceContext*	_context = nullptr;
		D3D11RenderStateManager	_renderStateManager;
	};
}



