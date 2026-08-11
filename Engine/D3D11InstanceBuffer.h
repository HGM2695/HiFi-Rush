#pragma once

#include "InstanceBuffer.h"
#include <wrl/client.h>

struct ID3D11Buffer;
struct ID3D11Device;

namespace gm
{
	class D3D11GraphicsResourceFactory;

	struct D3D11InstanceBufferDesc
	{
		ID3D11Device* device = nullptr;
		uint32 stride = 0;
		uint32 capacity = 0;
	};

	class D3D11InstanceBuffer final : public InstanceBuffer
	{
		friend class D3D11GraphicsResourceFactory;

	public:
		virtual ~D3D11InstanceBuffer() = default;

		ID3D11Buffer* GetNativeBuffer() const { return _buffer.Get(); }

	private:
		static std::unique_ptr<InstanceBuffer> Create(const D3D11InstanceBufferDesc& desc);

		explicit D3D11InstanceBuffer(const D3D11InstanceBufferDesc& desc);
		bool Initialize(const D3D11InstanceBufferDesc& desc);

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};
}
