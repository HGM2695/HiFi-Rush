#pragma once

#include "Texture.h"
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11TextureDesc : TextureDesc
	{
		ID3D11Device* device = nullptr;
	};

	class D3D11Texture final : public Texture
	{
		friend D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11Texture() = default;

		ID3D11ShaderResourceView* GetShaderResourceView() const { return _shaderResourceView.Get(); }

	private:
		static std::shared_ptr<Texture> Create(const D3D11TextureDesc& desc);

		D3D11Texture(const D3D11TextureDesc& desc, uint32 width, uint32 height, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView);

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	_shaderResourceView;
	};
}
