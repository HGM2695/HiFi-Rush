#pragma once

#include "IGraphicsCommandContext.h"

struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace gm
{
	class D3D11GraphicsCommandContext : public IGraphicsCommandContext
	{
	public:
		explicit D3D11GraphicsCommandContext(ID3D11DeviceContext* context);

		virtual void		SetPipelineState(const PipelineState& state) override;

		virtual void		SetPrimitiveTopology(PrimitiveTopology topology) override;
		virtual void		SetVertexShader(const Shader& shader) override;
		virtual void		SetPixelShader(const Shader& shader) override;
		virtual void		SetMesh(const Mesh& mesh) override;
		virtual void		SetMaterial(const Material& material) override;

		virtual void		SetTexture(uint32 slot, const Texture* texture) override;
		virtual void		SetSampler(uint32 slot, const Sampler* sampler) override;

		virtual void		SetConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer) override;
		virtual void		UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size) override;

		virtual void		DrawIndexed(uint32 indexCount) override;

	private:
		void BindVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout);
		void BindPixelShader(ID3D11PixelShader* pixelShader);
		void BindVertexBuffer(ID3D11Buffer* vertexBuffer, uint32 stride);
		void BindIndexBuffer(ID3D11Buffer* indexBuffer);
		void BindTexture(uint32 slot, ID3D11ShaderResourceView* shaderResourceView);
		void BindSampler(uint32 slot, ID3D11SamplerState* samplerState);

		ID3D11DeviceContext* _context = nullptr;
	};
}



