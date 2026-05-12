#pragma once

#include "Resource.h"
#include "GraphicsTypes.h"

namespace gm
{
	struct MeshDesc
	{
		PrimitiveTopology	topology = PrimitiveTopology::TriangleList;

		const void*			vertexData = nullptr;
		uint32				vertexCount = 0;
		uint32				vertexStride = 0;

		const uint32*		indexData = nullptr;
		uint32				indexCount = 0;
	};

	class Mesh : public Resource
	{
	public:
		virtual ~Mesh() = default;

		static inline ResourceType Type = ResourceType::Mesh;
		virtual ResourceType GetType() const override { return Type; }
		
		PrimitiveTopology	GetTopology() const { return _topology; }
		uint32				GetVertexCount() const { return _vertexCount; }
		uint32				GetIndexCount() const { return _indexCount; }

	protected:
		Mesh(PrimitiveTopology topology, uint32 vertexCount, uint32 indexCount)	: _topology(topology), _vertexCount(vertexCount), _indexCount(indexCount) {}

	protected:
		PrimitiveTopology	_topology = PrimitiveTopology::TriangleList;
		uint32				_vertexCount{};
		uint32				_indexCount{};
	};
}


