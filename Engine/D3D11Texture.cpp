#include "D3D11Texture.h"
#include <Windows.h>
#include <d3d11.h>
#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>
#include <filesystem>

namespace gm
{
	D3D11Texture::D3D11Texture(const D3D11TextureDesc& desc, uint32 width, uint32 height, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView)
		: Texture(desc, width, height), _shaderResourceView(std::move(shaderResourceView)) {}

	std::shared_ptr<Texture> D3D11Texture::Create(const D3D11TextureDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.path.empty() == false, nullptr, "텍스처 파일 경로가 비어 있습니다.");

		const std::wstring ext = std::filesystem::path(desc.path).extension().wstring();
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

		HRESULT hr{};
		if (ext == L".dds")
		{
			hr = DirectX::CreateDDSTextureFromFile(desc.device, desc.path.c_str(), resource.GetAddressOf(), shaderResourceView.GetAddressOf());
		}
		else if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp")
		{
			hr = DirectX::CreateWICTextureFromFile(desc.device, desc.path.c_str(), resource.GetAddressOf(), shaderResourceView.GetAddressOf());
			GM_LOG("%ls 확장자로 Texture를 생성했습니다. 런타임 리소스는 dds 변환을 권장합니다.", ext.c_str());
		}
		else
		{
			GM_ASSERT_RETURN_VAL(false, nullptr, "%ls 확장자는 지원하지 않습니다.", ext.c_str());
		}

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11Texture 생성에 실패했습니다.");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
		hr = resource.As(&texture2D);
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "Texture2D 변환에 실패했습니다.");

		D3D11_TEXTURE2D_DESC textureDesc{};
		texture2D->GetDesc(&textureDesc);
		
		return std::shared_ptr<Texture>(new D3D11Texture(desc, textureDesc.Width, textureDesc.Height, std::move(shaderResourceView)));
	}
}