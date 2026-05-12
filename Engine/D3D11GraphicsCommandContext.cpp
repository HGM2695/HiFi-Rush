#include "D3D11GraphicsCommandContext.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11TypeConverter.h"
#include "D3D11PipelineState.h"
#include "Shader.h"
#include <d3d11.h>

namespace gm
{
	D3D11GraphicsCommandContext::D3D11GraphicsCommandContext(ID3D11DeviceContext* context) : _context(context) {}

	void D3D11GraphicsCommandContext::SetPipelineState(const PipelineState& state)
	{
		const D3D11PipelineState& d3d11State = static_cast<const D3D11PipelineState&>(state);

		SetPrimitiveTopology(d3d11State.GetTopology());
		SetVertexShader(*d3d11State.GetVertexShader());
		SetPixelShader(*d3d11State.GetPixelShader());
		// depthStencil, rasterizer, blend state도 추후 대응해야 합니다.
	}

	void D3D11GraphicsCommandContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		_context->IASetPrimitiveTopology(ToD3D11PrimitiveTopology(topology));
	}

	void D3D11GraphicsCommandContext::SetVertexShader(const Shader& shader)
	{
		GM_ASSERT_RETURN(shader.GetStage() == ShaderStage::Vertex, "VertexShader가 아닌 셰이더를 VertexShader 슬롯에 바인딩할 수 없습니다.");
		const D3D11VertexShader& vertexShader = static_cast<const D3D11VertexShader&>(shader);
		BindVertexShader(vertexShader.GetNativeShader(), vertexShader.GetInputLayout());
	}

	void D3D11GraphicsCommandContext::SetPixelShader(const Shader& shader)
	{
		GM_ASSERT_RETURN(shader.GetStage() == ShaderStage::Pixel, "PixelShader가 아닌 셰이더를 PixelShader 슬롯에 바인딩할 수 없습니다.");
		const D3D11PixelShader& pixelShader = static_cast<const D3D11PixelShader&>(shader);
		BindPixelShader(pixelShader.GetNativeShader());
	}

	void D3D11GraphicsCommandContext::SetMesh(Mesh& mesh)
	{
		D3D11Mesh& d3d11Mesh = static_cast<D3D11Mesh&>(mesh);
		BindVertexBuffer(d3d11Mesh.GetVertexBuffer(), d3d11Mesh.GetVertexStride());

		if (d3d11Mesh.GetIndexBuffer())
			BindIndexBuffer(d3d11Mesh.GetIndexBuffer());
	}

	void D3D11GraphicsCommandContext::DrawIndexed(uint32 indexCount)
	{
		_context->DrawIndexed(indexCount, 0, 0);
	}

	void D3D11GraphicsCommandContext::BindVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout)
	{
		GM_ASSERT_RETURN(vertexShader, "VertexShader가 유효하지 않습니다.");

		_context->IASetInputLayout(inputLayout);
		_context->VSSetShader(vertexShader, nullptr, 0);
	}

	void D3D11GraphicsCommandContext::BindPixelShader(ID3D11PixelShader* pixelShader)
	{
		GM_ASSERT_RETURN(pixelShader, "PixelShader가 유효하지 않습니다.");

		_context->PSSetShader(pixelShader, nullptr, 0);
	}

	void D3D11GraphicsCommandContext::BindVertexBuffer(ID3D11Buffer* vertexBuffer, uint32 stride)
	{
		GM_ASSERT_RETURN(vertexBuffer, "VertexBuffer가 유효하지 않습니다.");

		ID3D11Buffer* buffers[] = { vertexBuffer };
		UINT strides[] = { stride };
		UINT offsets[] = { 0 };

		_context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	}

	void D3D11GraphicsCommandContext::BindIndexBuffer(ID3D11Buffer* indexBuffer)
	{
		GM_ASSERT_RETURN(indexBuffer, "IndexBuffer가 유효하지 않습니다.");

		_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

}
