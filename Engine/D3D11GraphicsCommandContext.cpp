#include "D3D11GraphicsCommandContext.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11Texture.h"
#include "D3D11TypeConverter.h"
#include "Material.h"
#include "Shader.h"
#include <d3d11.h>

namespace gm
{
	D3D11GraphicsCommandContext::D3D11GraphicsCommandContext(ID3D11Device* device, ID3D11DeviceContext* context)
		: _device(device)
		, _context(context)
		, _renderStateManager(device)
	{
	}

	void D3D11GraphicsCommandContext::BindPrimitiveTopology(PrimitiveTopology topology)
	{
		_context->IASetPrimitiveTopology(ToD3D11PrimitiveTopology(topology));
	}

	void D3D11GraphicsCommandContext::BindRasterizerState(const RasterizerDesc& desc)
	{
		BindNativeRasterizerState(_renderStateManager.GetRasterizerState(desc));
	}

	void D3D11GraphicsCommandContext::BindDepthStencilState(const DepthStencilDesc& desc)
	{
		BindNativeDepthStencilState(_renderStateManager.GetDepthStencilState(desc));
	}

	void D3D11GraphicsCommandContext::BindBlendState(const BlendDesc& desc)
	{
		BindNativeBlendState(_renderStateManager.GetBlendState(desc));
	}

	void D3D11GraphicsCommandContext::BindVertexShader(const Shader& shader)
	{
		GM_ASSERT_RETURN(shader.GetStage() == ShaderStage::Vertex, "VertexShader가 아닌 Shader를 VertexShader로 바인딩할 수 없습니다.");

		const D3D11VertexShader& vertexShader = static_cast<const D3D11VertexShader&>(shader);
		BindNativeVertexShader(vertexShader.GetNativeShader(), vertexShader.GetInputLayout());
	}

	void D3D11GraphicsCommandContext::BindPixelShader(const Shader& shader)
	{
		GM_ASSERT_RETURN(shader.GetStage() == ShaderStage::Pixel, "PixelShader가 아닌 Shader를 PixelShader로 바인딩할 수 없습니다.");

		const D3D11PixelShader& pixelShader = static_cast<const D3D11PixelShader&>(shader);
		BindNativePixelShader(pixelShader.GetNativeShader());
	}

	void D3D11GraphicsCommandContext::BindMesh(const Mesh& mesh)
	{
		const D3D11Mesh& d3d11Mesh = static_cast<const D3D11Mesh&>(mesh);
		BindNativeVertexBuffer(d3d11Mesh.GetVertexBuffer(), d3d11Mesh.GetVertexStride());

		if (d3d11Mesh.GetIndexBuffer())
			BindNativeIndexBuffer(d3d11Mesh.GetIndexBuffer());
	}

	void D3D11GraphicsCommandContext::BindTexture(uint32 slot, const Texture* texture)
	{
		const D3D11Texture* d3d11Texture = static_cast<const D3D11Texture*>(texture);
		ID3D11ShaderResourceView* srv = d3d11Texture ? d3d11Texture->GetShaderResourceView() : nullptr;
		BindNativeTexture(slot, srv);
	}

	void D3D11GraphicsCommandContext::BindSampler(uint32 slot, const SamplerDesc* samplerDesc)
	{
		ID3D11SamplerState* nativeSampler = samplerDesc ? _renderStateManager.GetSamplerState(*samplerDesc) : nullptr;
		BindNativeSampler(slot, nativeSampler);
	}

	void D3D11GraphicsCommandContext::BindConstantBuffer(ShaderStage stage, uint32 slot, const ConstantBuffer* cbuffer)
	{
		const D3D11ConstantBuffer* d3dBuffer = static_cast<const D3D11ConstantBuffer*>(cbuffer);
		ID3D11Buffer* nativeBuffer = d3dBuffer ? d3dBuffer->GetNativeBuffer() : nullptr;

		switch (stage)
		{
		case ShaderStage::Vertex:
			_context->VSSetConstantBuffers(slot, 1, &nativeBuffer);
			break;

		case ShaderStage::Pixel:
			_context->PSSetConstantBuffers(slot, 1, &nativeBuffer);
			break;

		default:
			GM_ASSERT_RETURN(false, "지원하지 않는 ShaderStage입니다.");
		}
	}

	void D3D11GraphicsCommandContext::UpdateConstantBuffer(ConstantBuffer& buffer, const void* data, uint32 size)
	{
		GM_ASSERT_RETURN(data, "상수 버퍼 업데이트 데이터가 nullptr입니다.");
		GM_ASSERT_RETURN(size == buffer.GetSize(), "상수 버퍼 사이즈와 업데이트 요청 사이즈가 다릅니다.");

		D3D11ConstantBuffer& d3dBuffer = static_cast<D3D11ConstantBuffer&>(buffer);
		_context->UpdateSubresource(d3dBuffer.GetNativeBuffer(), 0, nullptr, data, 0, 0);
	}

	void D3D11GraphicsCommandContext::BindMaterial(const Material& material)
	{
		std::shared_ptr<Shader> vertexShader = material.GetVertexShader();
		std::shared_ptr<Shader> pixelShader = material.GetPixelShader();
		GM_ASSERT_RETURN(vertexShader, "Material에 VertexShader가 없습니다.");
		GM_ASSERT_RETURN(pixelShader, "Material에 PixelShader가 없습니다.");

		BindVertexShader(*vertexShader);
		BindPixelShader(*pixelShader);
		BindPrimitiveTopology(material.GetTopology());
		BindRasterizerState(material.GetRasterizerDesc());
		BindDepthStencilState(material.GetDepthStencilDesc());
		BindBlendState(material.GetBlendDesc());

		for (uint32 i = 0; i < TextureSlotCount; ++i)
		{
			const TextureSlot slot = ToTextureSlot(i);
			BindTexture(i, material.GetTexture(slot).get());
		}

		for (uint32 i = 0; i < TextureSlotCount; ++i)
		{
			const TextureSlot slot = ToTextureSlot(i);
			BindSampler(i, &material.GetSamplerDesc(slot));
		}
	}

	void D3D11GraphicsCommandContext::DrawIndexed(uint32 indexCount)
	{
		DrawIndexed(indexCount, 0, 0);
	}

	void D3D11GraphicsCommandContext::DrawIndexed(uint32 indexCount, uint32 startIndexLocation, int32 baseVertexLocation)
	{
		_context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void D3D11GraphicsCommandContext::BindNativeVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout)
	{
		GM_ASSERT_RETURN(vertexShader, "VertexShader가 유효하지 않습니다.");

		_context->IASetInputLayout(inputLayout);
		_context->VSSetShader(vertexShader, nullptr, 0);
	}

	void D3D11GraphicsCommandContext::BindNativePixelShader(ID3D11PixelShader* pixelShader)
	{
		GM_ASSERT_RETURN(pixelShader, "PixelShader가 유효하지 않습니다.");
		_context->PSSetShader(pixelShader, nullptr, 0);
	}

	void D3D11GraphicsCommandContext::BindNativeVertexBuffer(ID3D11Buffer* vertexBuffer, uint32 stride)
	{
		GM_ASSERT_RETURN(vertexBuffer, "VertexBuffer가 유효하지 않습니다.");

		ID3D11Buffer* buffers[] = { vertexBuffer };
		UINT strides[] = { stride };
		UINT offsets[] = { 0 };
		_context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	}

	void D3D11GraphicsCommandContext::BindNativeIndexBuffer(ID3D11Buffer* indexBuffer)
	{
		GM_ASSERT_RETURN(indexBuffer, "IndexBuffer가 유효하지 않습니다.");
		_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	void D3D11GraphicsCommandContext::BindNativeTexture(uint32 slot, ID3D11ShaderResourceView* shaderResourceView)
	{
		_context->PSSetShaderResources(slot, 1, &shaderResourceView);
	}

	void D3D11GraphicsCommandContext::BindNativeSampler(uint32 slot, ID3D11SamplerState* samplerState)
	{
		_context->PSSetSamplers(slot, 1, &samplerState);
	}

	void D3D11GraphicsCommandContext::BindNativeRasterizerState(ID3D11RasterizerState* rasterizerState)
	{
		_context->RSSetState(rasterizerState);
	}

	void D3D11GraphicsCommandContext::BindNativeDepthStencilState(ID3D11DepthStencilState* depthStencilState)
	{
		_context->OMSetDepthStencilState(depthStencilState, 0);
	}

	void D3D11GraphicsCommandContext::BindNativeBlendState(ID3D11BlendState* blendState)
	{
		const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);
	}
}
