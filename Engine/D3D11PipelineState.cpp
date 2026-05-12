#include "D3D11PipelineState.h"
#include "D3D11TypeConverter.h"
#include <d3d11.h>

namespace gm
{
	D3D11PipelineState::D3D11PipelineState(const D3D11PipelineStateDesc& desc) : PipelineState(desc) {}

	std::shared_ptr<PipelineState> D3D11PipelineState::Create(const D3D11PipelineStateDesc& desc)
	{
		auto pipelineState = std::shared_ptr<D3D11PipelineState>(new D3D11PipelineState(desc));
		GM_ASSERT_RETURN_VAL(pipelineState->Initialize(desc), nullptr, "D3D11PipelineState 초기화 실패");
		return pipelineState;
	}

	bool D3D11PipelineState::Initialize(const D3D11PipelineStateDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, false, "D3D11 디바이스가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.vertexShader, false, "VertexShader가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.pixelShader, false, "PixelShader가 유효하지 않습니다.");

		GM_ASSERT_RETURN_VAL(CreateRasterizerState(desc.device, desc.rasterizerDesc), false, "RasterizerState 생성 실패");
		GM_ASSERT_RETURN_VAL(CreateDepthStencilState(desc.device, desc.depthStencilDesc), false, "DepthStencilState 생성 실패");
		GM_ASSERT_RETURN_VAL(CreateBlendState(desc.device, desc.blendDesc), false, "BlendState 생성 실패");
		return true;
	}

	bool D3D11PipelineState::CreateRasterizerState(ID3D11Device* device, const RasterizerDesc& desc)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = ToD3D11FillMode(desc.fillMode);
		rasterizerDesc.CullMode = ToD3D11CullMode(desc.cullMode);
		rasterizerDesc.FrontCounterClockwise = desc.frontCounterClockwise;
		rasterizerDesc.DepthClipEnable = desc.depthClipEnable;

		const HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, _rasterizerState.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "Rasterizer State 생성 실패");

		return true;
	}

	bool D3D11PipelineState::CreateDepthStencilState(ID3D11Device* device, const DepthStencilDesc& desc)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = desc.depthEnable;
		depthStencilDesc.DepthWriteMask = desc.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = ToD3D11ComparisonFunc(desc.depthFunc);
		depthStencilDesc.StencilEnable = desc.stencilEnable;

		const HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, _depthStencilState.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "Depth Stencil State 생성 실패");

		return true;
	}

	bool D3D11PipelineState::CreateBlendState(ID3D11Device* device, const BlendDesc& desc)
	{
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

		const HRESULT hr = device->CreateBlendState(&blendDesc, _blendState.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "Blend State 생성 실패");

		return true;
	}
}
