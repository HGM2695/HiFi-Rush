#pragma once

#include "PipelineState.h"
#include "GraphicsTypes.h"
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace gm
{
	struct D3D11PipelineStateDesc : PipelineStateDesc
	{
		ID3D11Device* device = nullptr;
	};

	class D3D11PipelineState : public PipelineState
	{
		friend class D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11PipelineState() = default;

		ID3D11RasterizerState*		GetRasterizerState() const { return _rasterizerState.Get(); }
		ID3D11DepthStencilState*	GetDepthStencilState() const { return _depthStencilState.Get(); }
		ID3D11BlendState*			GetBlendState() const { return _blendState.Get(); }

	private:
		static std::shared_ptr<PipelineState> Create(const D3D11PipelineStateDesc& desc);

		D3D11PipelineState(const D3D11PipelineStateDesc& desc);

		bool Initialize(const D3D11PipelineStateDesc& desc);
		bool CreateRasterizerState(ID3D11Device* device, const RasterizerDesc& desc);
		bool CreateDepthStencilState(ID3D11Device* device, const DepthStencilDesc& desc);
		bool CreateBlendState(ID3D11Device* device, const BlendDesc& desc);

	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depthStencilState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState;
	};
}