#include "D3D11InstanceBuffer.h"
#include <d3d11.h>
#include <limits>

namespace gm
{
	std::unique_ptr<InstanceBuffer> D3D11InstanceBuffer::Create(const D3D11InstanceBufferDesc& desc)
	{
		auto buffer = std::unique_ptr<D3D11InstanceBuffer>(new D3D11InstanceBuffer(desc));
		GM_ASSERT_RETURN_VAL(buffer->Initialize(desc), nullptr, "D3D11 Instance Buffer 생성에 실패했습니다.");
		return buffer;
	}

	D3D11InstanceBuffer::D3D11InstanceBuffer(const D3D11InstanceBufferDesc& desc)
		: InstanceBuffer(desc.stride, desc.capacity)
	{
	}

	bool D3D11InstanceBuffer::Initialize(const D3D11InstanceBufferDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, false, "D3D11 Device가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.stride > 0, false, "Instance Buffer Stride가 0입니다.");
		GM_ASSERT_RETURN_VAL(desc.capacity > 0, false, "Instance Buffer Capacity가 0입니다.");

		const uint64 byteWidth = static_cast<uint64>(desc.stride) * desc.capacity;
		GM_ASSERT_RETURN_VAL(byteWidth <= std::numeric_limits<uint32>::max(), false, "Instance Buffer 크기가 D3D11 제한을 초과했습니다.");

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<uint32>(byteWidth);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		const HRESULT hr = desc.device->CreateBuffer(&bufferDesc, nullptr, _buffer.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D3D11 Instance Buffer 생성에 실패했습니다.");
		return true;
	}
}
