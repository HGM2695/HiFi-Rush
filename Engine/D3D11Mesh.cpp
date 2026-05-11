#include "D3D11Mesh.h"

namespace gm
{
	std::shared_ptr<Mesh> D3D11Mesh::Create(const D3D11MeshDesc& desc)
	{
		auto mesh = std::shared_ptr<D3D11Mesh>(new D3D11Mesh(desc));
		GM_ASSERT_RETURN_VAL(mesh->Initialize(desc), nullptr, "D3D11 Mesh 생성 실패");

		return mesh;
	}

	D3D11Mesh::D3D11Mesh(const D3D11MeshDesc& desc)
		: Mesh(desc.topology, desc.vertexCount, desc.indexCount), _vertexStride(desc.vertexStride)
	{
	}

	bool D3D11Mesh::Initialize(const D3D11MeshDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.device, false, "D3D11 디바이스가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.vertexData, false, "버텍스 데이터가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(desc.vertexCount > 0, false, "버텍스 개수가 0입니다.");
		GM_ASSERT_RETURN_VAL(desc.vertexStride > 0, false, "버텍스 스트라이드가 0입니다.");

		GM_ASSERT_RETURN_VAL(CreateVertexBuffer(desc), false, "버텍스 버퍼 생성에 실패했습니다.");

		if (desc.indexData == nullptr || desc.indexCount == 0)
			return true;

		GM_ASSERT_RETURN_VAL(CreateIndexBuffer(desc), false, "인덱스 버퍼 생성에 실패했습니다.");

		return true;
	}

	bool D3D11Mesh::CreateVertexBuffer(const D3D11MeshDesc& desc)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = desc.vertexStride * desc.vertexCount;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = desc.vertexData;

		const HRESULT hr = desc.device->CreateBuffer(&bufferDesc, &initData, _vertexBuffer.GetAddressOf());
		
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "버텍스 버퍼 생성 실패");
		return true;
	}

	bool D3D11Mesh::CreateIndexBuffer(const D3D11MeshDesc& desc)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = desc.indexCount * sizeof(uint32);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = desc.indexData;

		const HRESULT hr = desc.device->CreateBuffer(&bufferDesc, &initData, _indexBuffer.GetAddressOf());

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "인덱스 버퍼 생성 실패");
		return true;
	}
}
