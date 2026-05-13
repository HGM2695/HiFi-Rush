#pragma once

#include "Sampler.h"
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11SamplerState;

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11SamplerDesc : SamplerDesc
	{
		ID3D11Device* device = nullptr;
	};

	class D3D11Sampler final : public Sampler
	{
		friend class D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11Sampler() = default;
		ID3D11SamplerState*				GetNativeSampler() const { return _samplerState.Get(); }

	private:
		D3D11Sampler(const D3D11SamplerDesc& desc);
		static std::shared_ptr<Sampler> Create(const D3D11SamplerDesc& desc);
		bool							Initialize(ID3D11Device* device);

	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState;
	};
}
