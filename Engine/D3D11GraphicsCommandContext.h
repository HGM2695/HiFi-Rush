#pragma once

#include "IGraphicsCommandContext.h"

struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

namespace gm
{
	class D3D11GraphicsCommandContext : public IGraphicsCommandContext
	{
	public:
		explicit D3D11GraphicsCommandContext(ID3D11DeviceContext* context);

		virtual void		SetPrimitiveTopology(PrimitiveTopology topology) override;
		virtual void		SetVertexShader(Shader& shader) override;
		virtual void		SetPixelShader(Shader& shader) override;
		virtual void		SetMesh(Mesh& mesh) override;

		virtual void		DrawIndexed(uint32 indexCount) override;

	private:
		void BindVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout);
		void BindPixelShader(ID3D11PixelShader* pixelShader);
		void BindVertexBuffer(ID3D11Buffer* vertexBuffer, uint32 stride);
		void BindIndexBuffer(ID3D11Buffer* indexBuffer);

		ID3D11DeviceContext* _context = nullptr;
	};
}



