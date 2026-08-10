#include "SkeletalMeshRenderPass.h"
#include "ConstantBuffer.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "Mesh.h"
#include "SkeletalMesh.h"

#include <algorithm>
#include <array>

namespace gm
{
	namespace
	{
		constexpr uint32 MaxSkinningBoneCount = 512;

		struct ObjectConstantVS
		{
			Matrix world;
		};

		struct CameraConstantVS
		{
			Matrix view;
			Matrix proj;
		};

		struct BonePaletteConstantVS
		{
			std::array<Matrix, MaxSkinningBoneCount> boneMatrices;
		};

		BonePaletteConstantVS BuildBonePalette(const MeshSection& section, const std::vector<Matrix>& boneModelMatrices)
		{
			BonePaletteConstantVS bonePalette{};
			for (Matrix& matrix : bonePalette.boneMatrices)
				matrix = Matrix::CreateScale(1.f);

			const uint32 paletteCount = std::min<uint32>(MaxSkinningBoneCount, static_cast<uint32>(section.boneIndices.size()));

			for (uint32 paletteIndex = 0; paletteIndex < paletteCount; ++paletteIndex)
			{
				const uint32 skeletonBoneIndex = section.boneIndices[paletteIndex];
				bonePalette.boneMatrices[paletteIndex] = section.offsetMatrices[paletteIndex] * boneModelMatrices[skeletonBoneIndex];
			}

			return bonePalette;
		}
	}

	SkeletalMeshRenderPass::SkeletalMeshRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	SkeletalMeshRenderPass::~SkeletalMeshRenderPass() = default;

	bool SkeletalMeshRenderPass::Initialize()
	{
		return true;
	}

	void SkeletalMeshRenderPass::Submit(const SkeletalMeshRenderItem& item)
	{
		if (item.skeletalMesh == nullptr || item.boneModelMatrices == nullptr)
			return;

		_items.push_back(item);
	}

	void SkeletalMeshRenderPass::Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum)
	{
		_constantBufferPool.ResetUsage();

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;

		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, cameraBuffer);

		for (const SkeletalMeshRenderItem& item : _items)
		{
			if (worldFrustum != nullptr && IsBoundingVolumeVisible(*worldFrustum, item.worldBounds) == false)
				continue;

			const SkeletalMesh& skeletalMesh = *item.skeletalMesh;
			const std::shared_ptr<Mesh>& mesh = skeletalMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = item.world;

			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
			_commandContext.BindMesh(*mesh);

			for (const MeshSection& section : skeletalMesh.GetSections())
			{
				if (section.indexCount == 0)
					continue;

				if (section.textureSetIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.textureSetIndex];
				if (material == nullptr || material->GetVertexShader() == nullptr || material->GetPixelShader() == nullptr)
					continue;

				BonePaletteConstantVS bonePalette = BuildBonePalette(section, *item.boneModelMatrices);
				ConstantBuffer* boneBuffer = _constantBufferPool.Acquire(sizeof(BonePaletteConstantVS));
				_commandContext.UpdateConstantBuffer(*boneBuffer, &bonePalette, sizeof(BonePaletteConstantVS));
				_commandContext.BindConstantBuffer(ShaderStage::Vertex, 2, boneBuffer);

				_commandContext.BindMaterial(*material);
				BindMaterialConstantData(*material);
				_commandContext.DrawIndexed(section.indexCount, section.indexStart, 0);
			}
		}

		Clear();
	}

	void SkeletalMeshRenderPass::Clear()
	{
		_items.clear();
	}

	void SkeletalMeshRenderPass::BindMaterialConstantData(const Material& material)
	{
		for (uint32 stageIndex = 0; stageIndex < ShaderStageCount; ++stageIndex)
		{
			const ShaderStage stage = static_cast<ShaderStage>(stageIndex);
			const Material::ConstantSlots& constantSlots = material.GetConstantSlots(stage);

			for (uint32 slot = 0; slot < MaxConstantBufferSlots; ++slot)
			{
				const Material::ConstantSlot& constantSlot = constantSlots[slot];
				if (constantSlot.IsValid() == false)
					continue;

				ConstantBuffer* buffer = _constantBufferPool.Acquire(constantSlot.Size());
				GM_ASSERT_RETURN(buffer, "Pool에서 Material ConstantBuffer를 가져오지 못했습니다.");

				_commandContext.UpdateConstantBuffer(*buffer, constantSlot.Data(), constantSlot.Size());
				_commandContext.BindConstantBuffer(stage, slot, buffer);
			}
		}
	}
}
