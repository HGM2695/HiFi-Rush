#include "D3D11Sampler.h"
#include "D3D11TypeConverter.h"
#include <d3d11.h>

namespace gm
{
	std::shared_ptr<Sampler> D3D11Sampler::Create(const D3D11SamplerDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, nullptr, "D3D11 디바이스가 유효하지 않습니다.");

		std::shared_ptr<D3D11Sampler> sampler(new D3D11Sampler(desc));
		GM_ASSERT_RETURN_VAL(sampler->Initialize(desc.device), nullptr, "D3D11 Sampler 생성에 실패했습니다.");

		return sampler;
	}

	D3D11Sampler::D3D11Sampler(const D3D11SamplerDesc& desc) : Sampler(desc) {}

	bool D3D11Sampler::Initialize(ID3D11Device* device)
	{
		const SamplerDesc& desc = GetDesc();

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = ToD3D11Filter(desc.filter);
		samplerDesc.AddressU = ToD3D11TextureAddressMode(desc.addressU);
		samplerDesc.AddressV = ToD3D11TextureAddressMode(desc.addressV);
		samplerDesc.AddressW = ToD3D11TextureAddressMode(desc.addressW);
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = desc.filter == TextureFilter::Anisotropic ? 16 : 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		const HRESULT hr = device->CreateSamplerState(&samplerDesc, _samplerState.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D3D11 SamplerState 생성에 실패했습니다.");

		return true;
	}
}
