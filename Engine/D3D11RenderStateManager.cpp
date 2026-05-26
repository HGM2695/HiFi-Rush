#include "D3D11RenderStateManager.h"
#include "D3D11TypeConverter.h"
#include "HashUtil.h"
#include <d3d11.h>

namespace gm
{
	D3D11RenderStateManager::D3D11RenderStateManager(ID3D11Device* device) : _device(device)
	{
		GM_ASSERT_RETURN(device, "D3D11 디바이스가 유효하지 않습니다.");
	}

	ID3D11RasterizerState* D3D11RenderStateManager::GetRasterizerState(const RasterizerDesc& desc)
	{
		const auto iter = _rasterizerStates.find(desc);
		if (iter != _rasterizerStates.end())
			return iter->second.Get();

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = ToD3D11FillMode(desc.fillMode);
		rasterizerDesc.CullMode = ToD3D11CullMode(desc.cullMode);
		rasterizerDesc.FrontCounterClockwise = desc.frontCounterClockwise;
		rasterizerDesc.DepthClipEnable = desc.depthClipEnable;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;
		const HRESULT hr = _device->CreateRasterizerState(&rasterizerDesc, state.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "Rasterizer State 생성에 실패했습니다.");

		ID3D11RasterizerState* statePtr = state.Get();
		_rasterizerStates.emplace(desc, std::move(state));
		return statePtr;
	}

	ID3D11DepthStencilState* D3D11RenderStateManager::GetDepthStencilState(const DepthStencilDesc& desc)
	{
		const auto iter = _depthStencilStates.find(desc);
		if (iter != _depthStencilStates.end())
			return iter->second.Get();

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = desc.depthEnable;
		depthStencilDesc.DepthWriteMask = desc.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = ToD3D11ComparisonFunc(desc.depthFunc);
		depthStencilDesc.StencilEnable = desc.stencilEnable;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
		const HRESULT hr = _device->CreateDepthStencilState(&depthStencilDesc, state.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "Depth Stencil State 생성에 실패했습니다.");

		ID3D11DepthStencilState* statePtr = state.Get();
		_depthStencilStates.emplace(desc, std::move(state));
		return statePtr;
	}

	ID3D11BlendState* D3D11RenderStateManager::GetBlendState(const BlendDesc& desc)
	{
		const auto iter = _blendStates.find(desc);
		if (iter != _blendStates.end())
			return iter->second.Get();

		D3D11_BLEND_DESC blendDesc{};
		D3D11_RENDER_TARGET_BLEND_DESC& rt = blendDesc.RenderTarget[0];
		rt.BlendEnable = desc.blendEnable;
		rt.SrcBlend = ToD3D11Blend(desc.srcBlend);
		rt.DestBlend = ToD3D11Blend(desc.destBlend);
		rt.BlendOp = ToD3D11BlendOp(desc.blendOp);
		rt.SrcBlendAlpha = D3D11_BLEND_ONE;
		rt.DestBlendAlpha = D3D11_BLEND_ZERO;
		rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		Microsoft::WRL::ComPtr<ID3D11BlendState> state;
		const HRESULT hr = _device->CreateBlendState(&blendDesc, state.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "Blend State 생성에 실패했습니다.");

		ID3D11BlendState* statePtr = state.Get();
		_blendStates.emplace(desc, std::move(state));
		return statePtr;
	}

	ID3D11SamplerState* D3D11RenderStateManager::GetSamplerState(const SamplerDesc& desc)
	{
		const auto iter = _samplerStates.find(desc);
		if (iter != _samplerStates.end())
			return iter->second.Get();

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = ToD3D11Filter(desc.filter);
		samplerDesc.AddressU = ToD3D11TextureAddressMode(desc.addressU);
		samplerDesc.AddressV = ToD3D11TextureAddressMode(desc.addressV);
		samplerDesc.AddressW = ToD3D11TextureAddressMode(desc.addressW);
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;
		const HRESULT hr = _device->CreateSamplerState(&samplerDesc, state.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "Sampler State 생성에 실패했습니다.");

		ID3D11SamplerState* statePtr = state.Get();
		_samplerStates.emplace(desc, std::move(state));
		return statePtr;
	}

	size_t D3D11RenderStateManager::RasterizerDescHasher::operator()(const RasterizerDesc& desc) const
	{
		size_t seed = 0;
		HashEnum(seed, desc.fillMode);
		HashEnum(seed, desc.cullMode);
		HashValue(seed, desc.frontCounterClockwise);
		HashValue(seed, desc.depthClipEnable);
		return seed;
	}

	size_t D3D11RenderStateManager::DepthStencilDescHasher::operator()(const DepthStencilDesc& desc) const
	{
		size_t seed = 0;
		HashValue(seed, desc.depthEnable);
		HashValue(seed, desc.depthWriteEnable);
		HashEnum(seed, desc.depthFunc);
		HashValue(seed, desc.stencilEnable);
		return seed;
	}

	size_t D3D11RenderStateManager::BlendDescHasher::operator()(const BlendDesc& desc) const
	{
		size_t seed = 0;
		HashValue(seed, desc.blendEnable);
		HashEnum(seed, desc.srcBlend);
		HashEnum(seed, desc.destBlend);
		HashEnum(seed, desc.blendOp);
		return seed;
	}

	size_t D3D11RenderStateManager::SamplerDescHasher::operator()(const SamplerDesc& desc) const
	{
		size_t seed = 0;
		HashEnum(seed, desc.filter);
		HashEnum(seed, desc.addressU);
		HashEnum(seed, desc.addressV);
		HashEnum(seed, desc.addressW);
		return seed;
	}
}
