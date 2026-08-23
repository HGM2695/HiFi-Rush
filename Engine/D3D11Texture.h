#pragma once

#include "Texture.h"
#include <vector>
#include <wrl/client.h>

struct ID3D11DepthStencilView;
struct ID3D11Device;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11TextureLoadDesc : TextureLoadDesc
	{
		ID3D11Device* device = nullptr;
	};

	struct D3D11TextureDesc : TextureDesc
	{
		ID3D11Device* device = nullptr;
	};

	class D3D11Texture final : public Texture
	{
		friend D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11Texture() = default;

		ID3D11RenderTargetView*		GetRenderTargetView() const { return _renderTargetView.Get(); }
		ID3D11DepthStencilView*		GetDepthStencilView() const { return _depthStencilView.Get(); }
		ID3D11DepthStencilView*		GetDepthStencilView(uint32 arraySlice) const;
		ID3D11ShaderResourceView*	GetShaderResourceView() const { return _shaderResourceView.Get(); }

	private:
		static std::shared_ptr<Texture> Load(const D3D11TextureLoadDesc& desc);
		static std::unique_ptr<Texture> Create(const D3D11TextureDesc& desc);
		D3D11Texture(uint32 width, uint32 height, uint32 arraySize, TextureFormat format, TextureBindUsage bindUsage) : Texture(width, height, arraySize, format, bindUsage) {}

	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D>						_texture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>				_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>				_depthStencilView;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> _depthStencilSliceViews;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>			_shaderResourceView;
	};
}
