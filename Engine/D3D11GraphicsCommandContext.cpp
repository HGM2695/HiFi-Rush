#include "D3D11GraphicsCommandContext.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11InstanceBuffer.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11Texture.h"
#include "D3D11TypeConverter.h"
#include "Material.h"
#include "Shader.h"
#include <d3d11.h>
#include <array>
#include <cstring>
#include <vector>

namespace gm
{
	D3D11GraphicsCommandContext::D3D11GraphicsCommandContext(D3D11GraphicsDevice& graphicsDevice)
		: _graphicsDevice(graphicsDevice)
		, _device(graphicsDevice.GetNativeDevice())
		, _context(graphicsDevice.GetImmediateContext())
		, _renderStateManager(_device)
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

	void D3D11GraphicsCommandContext::UnbindPixelShader()
	{
		_context->PSSetShader(nullptr, nullptr, 0);
	}

	void D3D11GraphicsCommandContext::BindMesh(const Mesh& mesh)
	{
		const D3D11Mesh& d3d11Mesh = static_cast<const D3D11Mesh&>(mesh);
		BindNativeVertexBuffer(0, d3d11Mesh.GetVertexBuffer(), d3d11Mesh.GetVertexStride());

		if (d3d11Mesh.GetIndexBuffer())
			BindNativeIndexBuffer(d3d11Mesh.GetIndexBuffer());
	}

	void D3D11GraphicsCommandContext::BindInstanceBuffer(const InstanceBuffer& buffer)
	{
		const D3D11InstanceBuffer& d3d11Buffer = static_cast<const D3D11InstanceBuffer&>(buffer);
		BindNativeVertexBuffer(1, d3d11Buffer.GetNativeBuffer(), d3d11Buffer.GetStride());
	}

	void D3D11GraphicsCommandContext::BindShaderTexture(uint32 slot, const Texture* texture)
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

	void D3D11GraphicsCommandContext::UnbindShaderTextures(uint32 startSlot, uint32 count)
	{
		if (count == 0)
			return;

		std::vector<ID3D11ShaderResourceView*> nullResources(count, nullptr);
		_context->PSSetShaderResources(startSlot, count, nullResources.data());
	}

	void D3D11GraphicsCommandContext::BindBackBuffer()
	{
		ID3D11RenderTargetView* renderTargetView = _graphicsDevice.GetRenderTargetView();
		ID3D11DepthStencilView* depthStencilView = _graphicsDevice.GetDepthStencilView();
		_context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	}

	void D3D11GraphicsCommandContext::BindRenderTarget(const Texture* renderTexture, const Texture* depthTexture)
	{
		ID3D11RenderTargetView* renderTargetView = renderTexture ? static_cast<const D3D11Texture*>(renderTexture)->GetRenderTargetView() : nullptr;
		ID3D11DepthStencilView* depthStencilView = depthTexture ? static_cast<const D3D11Texture*>(depthTexture)->GetDepthStencilView() : nullptr;
		_context->OMSetRenderTargets(renderTexture ? 1u : 0u, renderTexture ? &renderTargetView : nullptr, depthStencilView);
	}

	void D3D11GraphicsCommandContext::BindRenderTargets(const std::vector<const Texture*>& renderTextures, const Texture* depthTexture)
	{
		GM_ASSERT_RETURN(renderTextures.size() <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "동시에 바인딩할 수 있는 Render Target 개수를 초과했습니다.");
		std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargetViews{};
		for (size_t index = 0; index < renderTextures.size(); ++index)
			renderTargetViews[index] = static_cast<const D3D11Texture*>(renderTextures[index])->GetRenderTargetView();

		ID3D11DepthStencilView* depthStencilView = depthTexture ? static_cast<const D3D11Texture*>(depthTexture)->GetDepthStencilView() : nullptr;
		_context->OMSetRenderTargets(static_cast<uint32>(renderTextures.size()), renderTextures.empty() ? nullptr : renderTargetViews.data(), depthStencilView);
	}

	void D3D11GraphicsCommandContext::BindDepthStencilSlice(const Texture& depthTexture, uint32 arraySlice)
	{
		const D3D11Texture& d3d11Texture = static_cast<const D3D11Texture&>(depthTexture);
		ID3D11DepthStencilView* depthStencilView = d3d11Texture.GetDepthStencilView(arraySlice);
		_context->OMSetRenderTargets(0, nullptr, depthStencilView);
	}

	void D3D11GraphicsCommandContext::ClearBackBuffer(const Color& color, float depth, uint8 stencil)
	{
		ID3D11RenderTargetView* renderTargetView = _graphicsDevice.GetRenderTargetView();
		ID3D11DepthStencilView* depthStencilView = _graphicsDevice.GetDepthStencilView();
		_context->ClearRenderTargetView(renderTargetView, color);
		_context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}

	void D3D11GraphicsCommandContext::ClearRenderTarget(Texture& renderTexture, const Color& color)
	{
		D3D11Texture& d3d11Texture = static_cast<D3D11Texture&>(renderTexture);
		_context->ClearRenderTargetView(d3d11Texture.GetRenderTargetView(), color);
	}

	void D3D11GraphicsCommandContext::ClearDepthStencil(Texture& depthTexture, float depth, uint8 stencil)
	{
		D3D11Texture& d3d11Texture = static_cast<D3D11Texture&>(depthTexture);
		const uint32 clearFlags = depthTexture.GetFormat() == TextureFormat::Depth24Stencil8 ? D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL : D3D11_CLEAR_DEPTH;
		_context->ClearDepthStencilView(d3d11Texture.GetDepthStencilView(), clearFlags, depth, stencil);
	}

	void D3D11GraphicsCommandContext::ClearDepthStencilSlice(Texture& depthTexture, uint32 arraySlice, float depth, uint8 stencil)
	{
		D3D11Texture& d3d11Texture = static_cast<D3D11Texture&>(depthTexture);
		const uint32 clearFlags = depthTexture.GetFormat() == TextureFormat::Depth24Stencil8 ? D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL : D3D11_CLEAR_DEPTH;
		_context->ClearDepthStencilView(d3d11Texture.GetDepthStencilView(arraySlice), clearFlags, depth, stencil);
	}

	void D3D11GraphicsCommandContext::SetViewport(const Viewport& viewport)
	{
		D3D11_VIEWPORT d3d11Viewport{};
		d3d11Viewport.TopLeftX = viewport.x;
		d3d11Viewport.TopLeftY = viewport.y;
		d3d11Viewport.Width = viewport.width;
		d3d11Viewport.Height = viewport.height;
		d3d11Viewport.MinDepth = viewport.minDepth;
		d3d11Viewport.MaxDepth = viewport.maxDepth;
		_context->RSSetViewports(1, &d3d11Viewport);
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

	bool D3D11GraphicsCommandContext::UpdateInstanceBuffer(InstanceBuffer& buffer, const void* data, uint32 instanceCount)
	{
		GM_ASSERT_RETURN_VAL(data, false, "Instance Buffer 업데이트 데이터가 nullptr입니다.");
		GM_ASSERT_RETURN_VAL(instanceCount > 0, false, "Instance 개수가 0입니다.");
		GM_ASSERT_RETURN_VAL(instanceCount <= buffer.GetCapacity(), false, "Instance 개수가 Buffer Capacity를 초과했습니다.");

		D3D11InstanceBuffer& d3d11Buffer = static_cast<D3D11InstanceBuffer&>(buffer);
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		const HRESULT hr = _context->Map(d3d11Buffer.GetNativeBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D3D11 Instance Buffer Map에 실패했습니다.");

		const size_t dataSize = static_cast<size_t>(buffer.GetStride()) * instanceCount;
		std::memcpy(mappedResource.pData, data, dataSize);
		_context->Unmap(d3d11Buffer.GetNativeBuffer(), 0);
		return true;
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
			BindShaderTexture(i, material.GetTexture(slot).get());
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

	void D3D11GraphicsCommandContext::DrawIndexedInstanced(
		uint32 indexCount,
		uint32 instanceCount,
		uint32 startIndexLocation,
		int32 baseVertexLocation,
		uint32 startInstanceLocation)
	{
		_context->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
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

	void D3D11GraphicsCommandContext::BindNativeVertexBuffer(uint32 slot, ID3D11Buffer* vertexBuffer, uint32 stride)
	{
		GM_ASSERT_RETURN(vertexBuffer, "VertexBuffer가 유효하지 않습니다.");

		ID3D11Buffer* buffers[] = { vertexBuffer };
		UINT strides[] = { stride };
		UINT offsets[] = { 0 };
		_context->IASetVertexBuffers(slot, 1, buffers, strides, offsets);
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
