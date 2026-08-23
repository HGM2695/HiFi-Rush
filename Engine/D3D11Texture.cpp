#include "D3D11Texture.h"
#include "D3D11TypeConverter.h"
#include <Windows.h>
#include <d3d11.h>
#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>
#include <filesystem>

namespace gm
{
	ID3D11DepthStencilView* D3D11Texture::GetDepthStencilView(uint32 arraySlice) const
	{
		GM_ASSERT_RETURN_VAL(arraySlice < _depthStencilSliceViews.size(), nullptr, "Depth Stencil Array Slice가 유효하지 않습니다. slice=%u", arraySlice);
		return _depthStencilSliceViews[arraySlice].Get();
	}

	std::shared_ptr<Texture> D3D11Texture::Load(const D3D11TextureLoadDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.path.empty() == false, nullptr, "Texture 파일 경로가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(desc.device, nullptr, "D3D11 Texture 로드에 필요한 Device가 없습니다.");
		GM_ASSERT_RETURN_VAL(desc.colorSpace < TextureColorSpace::Count, nullptr, "지원하지 않는 Texture Color Space입니다.");

		const std::wstring ext = std::filesystem::path(desc.path).extension().wstring();
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		HRESULT hr{};
		if (ext == L".dds")
		{
			const DirectX::DDS_LOADER_FLAGS loadFlags = desc.colorSpace == TextureColorSpace::SRGB ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB;
			hr = DirectX::CreateDDSTextureFromFileEx(desc.device, desc.path.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, loadFlags, resource.GetAddressOf(), shaderResourceView.GetAddressOf());
		}
		else if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp")
		{
			const DirectX::WIC_LOADER_FLAGS loadFlags = desc.colorSpace == TextureColorSpace::SRGB ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_IGNORE_SRGB;
			hr = DirectX::CreateWICTextureFromFileEx(desc.device, desc.path.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, loadFlags, resource.GetAddressOf(), shaderResourceView.GetAddressOf());
			GM_LOG("%ls 확장자로 Texture를 생성했습니다. 런타임 리소스는 dds 변환을 권장합니다.", ext.c_str());
		}
		else
		{
			GM_ASSERT_RETURN_VAL(false, nullptr, "%ls 확장자는 지원하지 않습니다.", ext.c_str());
		}

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Texture 로드에 실패했습니다.");
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		hr = resource.As(&texture);
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "로드한 Resource의 Texture2D 변환에 실패했습니다.");

		D3D11_TEXTURE2D_DESC textureDesc{};
		texture->GetDesc(&textureDesc);
		std::shared_ptr<D3D11Texture> loadedTexture(new D3D11Texture(textureDesc.Width, textureDesc.Height, textureDesc.ArraySize, TextureFormat::Unknown, TextureBindUsage::ShaderResource));
		loadedTexture->_texture = std::move(texture);
		loadedTexture->_shaderResourceView = std::move(shaderResourceView);
		return loadedTexture;
	}

	std::unique_ptr<Texture> D3D11Texture::Create(const D3D11TextureDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, nullptr, "D3D11 Texture 생성에 필요한 Device가 없습니다.");
		GM_ASSERT_RETURN_VAL(desc.width > 0 && desc.height > 0, nullptr, "Texture 크기는 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(desc.arraySize > 0, nullptr, "Texture Array 크기는 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(desc.format > TextureFormat::Unknown && desc.format < TextureFormat::Count, nullptr, "Texture 생성에 지원하지 않는 Texture Format입니다.");
		GM_ASSERT_RETURN_VAL(desc.bindUsage != TextureBindUsage::None, nullptr, "Texture에는 하나 이상의 Bind Usage가 필요합니다.");
		GM_ASSERT_RETURN_VAL(desc.initialData == nullptr || desc.arraySize == 1, nullptr, "Texture Array의 초기 데이터 생성은 지원하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.initialData == nullptr || desc.initialDataRowPitch > 0, nullptr, "Texture 초기 데이터에는 Row Pitch가 필요합니다.");

		const bool isDepthFormat = desc.format == TextureFormat::Depth24Stencil8 || desc.format == TextureFormat::Depth32Float;
		const bool isRenderTarget = HasTextureBindUsage(desc.bindUsage, TextureBindUsage::RenderTarget);
		const bool isDepthStencil = HasTextureBindUsage(desc.bindUsage, TextureBindUsage::DepthStencil);
		GM_ASSERT_RETURN_VAL(isDepthFormat == false || isRenderTarget == false, nullptr, "Depth Texture Format은 Render Target으로 사용할 수 없습니다.");
		GM_ASSERT_RETURN_VAL(isDepthFormat || isDepthStencil == false, nullptr, "Color Texture Format은 Depth Stencil로 사용할 수 없습니다.");

		std::unique_ptr<D3D11Texture> texture(new D3D11Texture(desc.width, desc.height, desc.arraySize, desc.format, desc.bindUsage));
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = desc.width;
		textureDesc.Height = desc.height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = desc.arraySize;
		textureDesc.Format = ToD3D11TextureFormat(desc.format);
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = ToD3D11BindFlags(desc.bindUsage);

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = desc.initialData;
		initialData.SysMemPitch = desc.initialDataRowPitch;
		HRESULT hr = desc.device->CreateTexture2D(&textureDesc, desc.initialData ? &initialData : nullptr, texture->_texture.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Texture 생성에 실패했습니다.");

		if (isRenderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
			renderTargetViewDesc.Format = ToD3D11RTVFormat(desc.format);
			renderTargetViewDesc.ViewDimension = desc.arraySize > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;
			if (desc.arraySize > 1)
			{
				renderTargetViewDesc.Texture2DArray.MipSlice = 0;
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
				renderTargetViewDesc.Texture2DArray.ArraySize = desc.arraySize;
			}
			hr = desc.device->CreateRenderTargetView(texture->_texture.Get(), &renderTargetViewDesc, texture->_renderTargetView.GetAddressOf());
			GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Render Target View 생성에 실패했습니다.");
		}

		if (isDepthStencil)
		{
			if (desc.arraySize == 1)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
				depthStencilViewDesc.Format = ToD3D11DSVFormat(desc.format);
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				hr = desc.device->CreateDepthStencilView(texture->_texture.Get(), &depthStencilViewDesc, texture->_depthStencilView.GetAddressOf());
				GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Depth Stencil View 생성에 실패했습니다.");
			}
			else
			{
				texture->_depthStencilSliceViews.resize(desc.arraySize);
				for (uint32 arraySlice = 0; arraySlice < desc.arraySize; ++arraySlice)
				{
					D3D11_DEPTH_STENCIL_VIEW_DESC sliceViewDesc{};
					sliceViewDesc.Format = ToD3D11DSVFormat(desc.format);
					sliceViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					sliceViewDesc.Texture2DArray.MipSlice = 0;
					sliceViewDesc.Texture2DArray.FirstArraySlice = arraySlice;
					sliceViewDesc.Texture2DArray.ArraySize = 1;
					hr = desc.device->CreateDepthStencilView(texture->_texture.Get(), &sliceViewDesc, texture->_depthStencilSliceViews[arraySlice].GetAddressOf());
					GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Depth Stencil Slice View 생성에 실패했습니다. slice=%u", arraySlice);
				}
			}
		}

		if (HasTextureBindUsage(desc.bindUsage, TextureBindUsage::ShaderResource))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
			shaderResourceViewDesc.Format = ToD3D11SRVFormat(desc.format);
			shaderResourceViewDesc.ViewDimension = desc.arraySize > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
			if (desc.arraySize > 1)
			{
				shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
				shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
				shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
				shaderResourceViewDesc.Texture2DArray.ArraySize = desc.arraySize;
			}
			else
			{
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
			}
			hr = desc.device->CreateShaderResourceView(texture->_texture.Get(), &shaderResourceViewDesc, texture->_shaderResourceView.GetAddressOf());
			GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 Shader Resource View 생성에 실패했습니다.");
		}

		return texture;
	}
}
