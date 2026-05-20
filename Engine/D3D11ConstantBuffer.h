#pragma once

#include "ConstantBuffer.h"
#include "GraphicsTypes.h"
#include <wrl/client.h>

struct ID3D11Device;
struct ID3D11Buffer;

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11ConstantBufferDesc : public ConstantBufferDesc
	{
		ID3D11Device* device = nullptr;
	};

	class D3D11ConstantBuffer final : public ConstantBuffer
	{
		friend D3D11GraphicsResourceFactory;

	public:
		ID3D11Buffer* GetNativeBuffer() const { return _buffer.Get(); }

	private:
		static std::unique_ptr<ConstantBuffer> Create(const D3D11ConstantBufferDesc& desc);

		D3D11ConstantBuffer(uint32 size, Microsoft::WRL::ComPtr<ID3D11Buffer> buffer) : ConstantBuffer(size), _buffer(std::move(buffer)) {}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};
}



