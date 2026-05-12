#pragma once

#include "GraphicsTypes.h"
#include "VertexLayout.h"
#include <d3d11.h>
#include <vector>

namespace gm
{
	D3D11_PRIMITIVE_TOPOLOGY				ToD3D11PrimitiveTopology(PrimitiveTopology topology);
	D3D11_FILL_MODE							ToD3D11FillMode(FillMode fillMode);
	D3D11_CULL_MODE							ToD3D11CullMode(CullMode cullMode);
	D3D11_COMPARISON_FUNC					ToD3D11ComparisonFunc(CompareFunc compareFunc);
	D3D11_BLEND								ToD3D11Blend(BlendFactor blendFactor);
	D3D11_BLEND_OP							ToD3D11BlendOp(BlendOp blendOp);
	DXGI_FORMAT								ToDXGIFormat(VertexElementFormat format);
	std::vector<D3D11_INPUT_ELEMENT_DESC>	ToD3D11InputElements(const VertexLayoutDesc& layoutDesc);
}
