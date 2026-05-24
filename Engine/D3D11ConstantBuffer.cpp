#include "D3D11ConstantBuffer.h"
#include <d3d11.h>

namespace gm
{
	std::unique_ptr<ConstantBuffer> D3D11ConstantBuffer::Create(const D3D11ConstantBufferDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, nullptr, "D3D11 디바이스가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.size > 0, nullptr, "상수 버퍼 크기가 0입니다.");
		GM_ASSERT_RETURN_VAL(desc.size % 16 == 0, nullptr, "상수 버퍼는 16바이트 정렬이 필요합니다.");

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = desc.size;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = desc.initialData;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		D3D11_SUBRESOURCE_DATA* pInitData = initData.pSysMem ? &initData : nullptr;
		const HRESULT hr = desc.device->CreateBuffer(&bufferDesc, pInitData, buffer.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), nullptr, "D3D11 상수 버퍼 생성에 실패했습니다.");

		return std::unique_ptr<ConstantBuffer>(new D3D11ConstantBuffer(desc.size, std::move(buffer)));
	}
}
