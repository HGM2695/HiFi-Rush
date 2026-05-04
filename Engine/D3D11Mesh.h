#pragma once

#include "Mesh.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace gm
{
	struct D3D11MeshDesc
	{
		ID3D11Device*		device{};
		PrimitiveTopology	topology = PrimitiveTopology::TriangleList;

		const void*			vertexData{};
		uint32				vertexCount{};
		uint32				vertexStride{};

		const uint32*		indexData{};
		uint32				indexCount{};
	};

	class D3D11Mesh : public Mesh
	{
	public:
		static std::shared_ptr<Mesh> Create(const D3D11MeshDesc& desc);
		virtual ~D3D11Mesh() = default;

	private:
		D3D11Mesh(const D3D11MeshDesc& desc);

		bool			Initialize(const D3D11MeshDesc& desc);
		bool			CreateVertexBuffer(const D3D11MeshDesc& desc);
		bool			CreateIndexBuffer(const D3D11MeshDesc& desc);

		uint32			GetVertexStride() const { return _vertexStride; }
		ID3D11Buffer*	GetVertexBuffer() const { return _vertexBuffer.Get(); }
		ID3D11Buffer*	GetIndexBuffer() const { return _indexBuffer.Get(); };

	private:
		uint32									_vertexStride{};
		Microsoft::WRL::ComPtr<ID3D11Buffer>	_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	_indexBuffer;

	};
}


