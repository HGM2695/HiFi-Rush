#pragma once

#include "GraphicsTypes.h"
#include <unordered_map>
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11SamplerState;

namespace gm
{
	class D3D11RenderStateManager
	{
	public:
		explicit D3D11RenderStateManager(ID3D11Device* device);

		ID3D11RasterizerState*		GetRasterizerState(const RasterizerDesc& desc);
		ID3D11DepthStencilState*	GetDepthStencilState(const DepthStencilDesc& desc);
		ID3D11BlendState*			GetBlendState(const BlendDesc& desc);
		ID3D11SamplerState*			GetSamplerState(const SamplerDesc& desc);

	private:
		struct DepthStencilDescHasher
		{
			size_t operator()(const DepthStencilDesc& desc) const;
		};

		struct RasterizerDescHasher
		{
			size_t operator()(const RasterizerDesc& desc) const;
		};

		struct BlendDescHasher
		{
			size_t operator()(const BlendDesc& desc) const;
		};

		struct SamplerDescHasher
		{
			size_t operator()(const SamplerDesc& desc) const;
		};

	private:
		ID3D11Device* _device = nullptr;

		std::unordered_map<DepthStencilDesc, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>, DepthStencilDescHasher>	_depthStencilStates;
		std::unordered_map<RasterizerDesc, Microsoft::WRL::ComPtr<ID3D11RasterizerState>, RasterizerDescHasher>			_rasterizerStates;
		std::unordered_map<BlendDesc, Microsoft::WRL::ComPtr<ID3D11BlendState>, BlendDescHasher>						_blendStates;
		std::unordered_map<SamplerDesc, Microsoft::WRL::ComPtr<ID3D11SamplerState>, SamplerDescHasher>					_samplerStates;
	};
}
