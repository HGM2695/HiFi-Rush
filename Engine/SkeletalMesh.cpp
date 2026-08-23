#include "SkeletalMesh.h"
#include "IGraphicsResourceFactory.h"
#include "Mesh.h"
#include "VertexTypes.h"

#include <utility>

namespace gm
{
	std::shared_ptr<SkeletalMesh> SkeletalMesh::Create(const ModelData& modelData, IGraphicsResourceFactory& resourceFactory)
	{
		GM_ASSERT_RETURN_VAL(modelData.type == ModelType::Skeletal, nullptr, "SkeletalMesh는 Skeletal 모델 데이터로만 생성할 수 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.bones.empty() == false, nullptr, "SkeletalMesh Bone 데이터가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.skinnedVertices.empty() == false, nullptr, "SkeletalMesh 정점 데이터가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.indices.empty() == false, nullptr, "SkeletalMesh 인덱스 데이터가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(modelData.localBounds.isValid, nullptr, "SkeletalMesh 로컬 바운드 데이터가 유효하지 않습니다.");

		MeshDesc meshDesc{};
		meshDesc.topology = PrimitiveTopology::TriangleList;
		meshDesc.vertexData = modelData.skinnedVertices.data();
		meshDesc.vertexCount = static_cast<uint32>(modelData.skinnedVertices.size());
		meshDesc.vertexStride = sizeof(VertexAnimationMesh);
		meshDesc.indexData = modelData.indices.data();
		meshDesc.indexCount = static_cast<uint32>(modelData.indices.size());

		std::shared_ptr<Mesh> mesh = resourceFactory.CreateMesh(meshDesc);
		GM_ASSERT_RETURN_VAL(mesh, nullptr, "SkeletalMesh 내부 Mesh 생성에 실패했습니다.");

		return std::shared_ptr<SkeletalMesh>(new SkeletalMesh(std::move(mesh), modelData));
	}

	SkeletalMesh::SkeletalMesh(std::shared_ptr<Mesh> mesh, const ModelData& modelData)
		: _preTransform(modelData.preTransform)
		, _localBounds(modelData.localBounds)
		, _castsShadow(modelData.castsShadow)
		, _mesh(std::move(mesh))
		, _sections(modelData.sections)
		, _materialSlots(modelData.materialSlots)
		, _bones(modelData.bones)
	{}

	const MeshSection* SkeletalMesh::GetSection(uint32 index) const
	{
		if (index >= _sections.size())
			return nullptr;

		return &_sections[index];
	}

	const MeshMaterialSlot* SkeletalMesh::GetMaterialSlot(uint32 index) const
	{
		if (index >= _materialSlots.size())
			return nullptr;

		return &_materialSlots[index];
	}
}
