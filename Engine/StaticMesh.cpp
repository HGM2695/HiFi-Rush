#include "StaticMesh.h"
#include "IGraphicsResourceFactory.h"
#include "Mesh.h"
#include "VertexTypes.h"

#include <utility>

namespace gm
{
	std::shared_ptr<StaticMesh> StaticMesh::Create(const ModelData& modelData, IGraphicsResourceFactory& resourceFactory)
	{
		GM_ASSERT_RETURN_VAL(modelData.type == ModelType::Static, nullptr, "StaticMesh는 Static 모델 데이터로만 생성할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.vertices.empty() == false, nullptr, "StaticMesh 정점 데이터가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.indices.empty() == false, nullptr, "StaticMesh 인덱스 데이터가 비어 있습니다.");

		MeshDesc meshDesc{};
		meshDesc.topology = PrimitiveTopology::TriangleList;
		meshDesc.vertexData = modelData.vertices.data();
		meshDesc.vertexCount = static_cast<uint32>(modelData.vertices.size());
		meshDesc.vertexStride = sizeof(VertexMesh);
		meshDesc.indexData = modelData.indices.data();
		meshDesc.indexCount = static_cast<uint32>(modelData.indices.size());

		std::shared_ptr<Mesh> mesh = resourceFactory.CreateMesh(meshDesc);
		GM_ASSERT_RETURN_VAL(mesh, nullptr, "StaticMesh 내부 Mesh 생성에 실패했습니다.");

		return std::shared_ptr<StaticMesh>(new StaticMesh(std::move(mesh), modelData));
	}

	StaticMesh::StaticMesh(std::shared_ptr<Mesh> mesh, const ModelData& modelData)
		: _mesh(std::move(mesh))
		, _sections(modelData.sections)
		, _textureSlots(modelData.textureSlots)
		, _preTransform(modelData.preTransform)
	{
		if (_sections.empty() && _mesh)
		{
			MeshSection section{};
			section.indexStart = 0;
			section.indexCount = _mesh->GetIndexCount();
			section.textureSlotIndex = 0;
			_sections.push_back(section);
		}
	}

	const MeshSection* StaticMesh::GetSection(uint32 index) const
	{
		if (index >= _sections.size())
			return nullptr;

		return &_sections[index];
	}

	const MeshTextureSlot* StaticMesh::GettextureSlot(uint32 index) const
	{
		if (index >= _textureSlots.size())
			return nullptr;

		return &_textureSlots[index];
	}
}
