#include "D3D11GraphicsCommandContext.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11TypeConverter.h"
#include "D3D11PipelineState.h"
#include "D3D11Texture.h"
#include "D3D11Sampler.h"
#include "D3D11ConstantBuffer.h"
#include "Shader.h"
#include "Material.h"
#include <d3d11.h>

namespace gm
{
	D3D11GraphicsCommandContext::D3D11GraphicsCommandContext(ID3D11DeviceContext* context) : _context(context) {}

	void D3D11GraphicsCommandContext::SetPipelineState(const PipelineState& state)
	{
		const D3D11PipelineState& d3d11State = static_cast<const D3D11PipelineState&>(state);

		Shader* vertexShader = d3d11State.GetVertexShader();
		Shader* pixelShader = d3d11State.GetPixelShader();
		GM_ASSERT_RETURN(vertexShader, "PipelineState에 VertexShader가 없습니다.");
		GM_ASSERT_RETURN(pixelShader, "PipelineState에 PixelShader가 없습니다.");

		SetPrimitiveTopology(d3d11State.GetTopology());
		SetVertexShader(*vertexShader);
		SetPixelShader(*pixelShader);
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

	void D3D11GraphicsCommandContext::SetMesh(const Mesh& mesh)
	{
		const D3D11Mesh& d3d11Mesh = static_cast<const D3D11Mesh&>(mesh);
		BindVertexBuffer(d3d11Mesh.GetVertexBuffer(), d3d11Mesh.GetVertexStride());

		if (d3d11Mesh.GetIndexBuffer())
			BindIndexBuffer(d3d11Mesh.GetIndexBuffer());
	}

	void D3D11GraphicsCommandContext::SetTexture(uint32 slot, const Texture* texture)
	{
		const D3D11Texture* d3d11Texture = static_cast<const D3D11Texture*>(texture);
		ID3D11ShaderResourceView* srv = d3d11Texture ? d3d11Texture->GetShaderResourceView() : nullptr;
		BindTexture(slot, srv);
	}

	void D3D11GraphicsCommandContext::SetSampler(uint32 slot, const Sampler* sampler)
	{
		const D3D11Sampler* d3d11Sampler = static_cast<const D3D11Sampler*>(sampler);
		ID3D11SamplerState* nativeSampler = d3d11Sampler ? d3d11Sampler->GetNativeSampler() : nullptr;
		BindSampler(slot, nativeSampler);
	}

	void D3D11GraphicsCommandContext::SetConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer)
	{
		const D3D11ConstantBuffer* d3dBuffer = static_cast<const D3D11ConstantBuffer*>(cbuffer);
		ID3D11Buffer* nativeBuffer = d3dBuffer ? d3dBuffer->GetNativeBuffer() : nullptr;

		switch (stage)
		{
		case gm::ShaderStage::Vertex:
			_context->VSSetConstantBuffers(slot, 1, &nativeBuffer);
			break;

		case gm::ShaderStage::Pixel:
			_context->PSSetConstantBuffers(slot, 1, &nativeBuffer);
			break;

		default:
			GM_ASSERT_RETURN(false, "지원하지 않는 ShaderStage입니다.");
		}
	}

	void D3D11GraphicsCommandContext::UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size)
	{
		GM_ASSERT_RETURN(data, "상수 버퍼 업데이트 데이터가 nullptr입니다.");
		GM_ASSERT_RETURN(size > 0, "상수 버퍼 업데이트 크기가 0입니다.");
		GM_ASSERT_RETURN(size <= buffer.GetSize(), "상수 버퍼 업데이트 크기가 버퍼 크기를 초과했습니다.");

		D3D11ConstantBuffer& d3dBuffer = static_cast<D3D11ConstantBuffer&>(buffer);
		_context->UpdateSubresource(d3dBuffer.GetNativeBuffer(), 0, nullptr, data, 0, 0);
	}

	void D3D11GraphicsCommandContext::SetMaterial(const Material& material)
	{
		PipelineState* pipelineState = material.GetPipelineState();
		GM_ASSERT_RETURN(pipelineState, "Material에 PipelineState가 없습니다.");
		SetPipelineState(*pipelineState);

		for (uint32 i = 0; i < MaxMaterialTextureSlots; ++i)
			SetTexture(i, material.GetTexture(i));
		
		for (uint32 i = 0; i < MaxMaterialSamplerSlots; ++i)
			SetSampler(i, material.GetSampler(i));
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

	void D3D11GraphicsCommandContext::BindTexture(uint32 slot, ID3D11ShaderResourceView* shaderResourceView)
	{
		// 비어있는 경우 nullptr 바인딩을 수행합니다.
		_context->PSSetShaderResources(slot, 1, &shaderResourceView);
	}

	void D3D11GraphicsCommandContext::BindSampler(uint32 slot, ID3D11SamplerState* samplerState)
	{
		// 비어있는 경우 nullptr 바인딩을 수행합니다.
		_context->PSSetSamplers(slot, 1, &samplerState);
	}
}
