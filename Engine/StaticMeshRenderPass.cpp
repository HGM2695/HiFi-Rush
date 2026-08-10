#include "StaticMeshRenderPass.h"
#include "ConstantBuffer.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "Mesh.h"
#include "StaticMesh.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

#include <algorithm>
#include <functional>

namespace gm
{
	namespace
	{
		struct ObjectConstantVS
		{
			Matrix world;
		};

		struct CameraConstantVS
		{
			Matrix view;
			Matrix proj;
		};

		struct StaticMeshBatchItem
		{
			StaticMeshBatchKey key{};
			Matrix world = Matrix::CreateScale(1.f);
		};

		bool CompareBatchItems(const StaticMeshBatchItem& lhs, const StaticMeshBatchItem& rhs)
		{
			if (lhs.key.materialStateHash != rhs.key.materialStateHash)
				return lhs.key.materialStateHash < rhs.key.materialStateHash;

			if (lhs.key.mesh != rhs.key.mesh)
				return std::less<const Mesh*>{}(lhs.key.mesh, rhs.key.mesh);

			if (lhs.key.indexStart != rhs.key.indexStart)
				return lhs.key.indexStart < rhs.key.indexStart;

			return lhs.key.indexCount < rhs.key.indexCount;
		}

		void AppendBatchItem(std::vector<StaticMeshRenderBatch>& batches, const StaticMeshBatchItem& item)
		{
			if (batches.empty() == false && batches.back().key == item.key)
			{
				batches.back().worlds.push_back(item.world);
				return;
			}

			StaticMeshRenderBatch batch{};
			batch.key = item.key;
			batch.worlds.push_back(item.world);
			batches.push_back(std::move(batch));
		}
	}

	bool StaticMeshBatchKey::operator==(const StaticMeshBatchKey& rhs) const
	{
		if (mesh != rhs.mesh || indexStart != rhs.indexStart || indexCount != rhs.indexCount ||
			materialStateHash != rhs.materialStateHash || material == nullptr || rhs.material == nullptr)
			return false;

		return material->HasSameRenderState(*rhs.material);
	}

	StaticMeshRenderPass::StaticMeshRenderPass(
		Resources& resources,
		IGraphicsCommandContext& commandContext,
		IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	StaticMeshRenderPass::~StaticMeshRenderPass() = default;

	bool StaticMeshRenderPass::Initialize()
	{
		return true;
	}

	void StaticMeshRenderPass::Submit(const StaticMeshRenderItem& item)
	{
		if (item.staticMesh == nullptr)
			return;

		_items.push_back(item);
	}

#if GM_ENABLE_DEBUG_TOOLS
	void StaticMeshRenderPass::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		for (const StaticMeshRenderItem& item : _items)
		{
			if (item.worldBounds.isValid)
				debugRenderer.RequestDrawBox(item.worldBounds.box, Colors::Green);
		}
	}
#endif

	void StaticMeshRenderPass::Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum)
	{
		_constantBufferPool.ResetUsage();
		_renderBatchList.clear();

#if GM_ENABLE_DEBUG_TOOLS
		_lastSubmittedItemCount = static_cast<uint32>(_items.size());
		_lastVisibleItemCount = 0;
		_lastCulledItemCount = 0;
#endif

		BuildRenderBatches(worldFrustum);

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;

		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, cameraBuffer);
		for (const StaticMeshRenderBatch& batch : _renderBatchList)
		{
			const StaticMeshBatchKey& key = batch.key;
			if (key.mesh == nullptr || key.material == nullptr || key.indexCount == 0)
				continue;

			_commandContext.BindMesh(*key.mesh);
			_commandContext.BindMaterial(*key.material);
			BindMaterialConstantData(*key.material);

			for (const Matrix& world : batch.worlds)
			{
				ObjectConstantVS objectConstantVS{};
				objectConstantVS.world = world;

				ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
				_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
				_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
				_commandContext.DrawIndexed(key.indexCount, key.indexStart, 0);
			}
		}

		Clear();
	}

	void StaticMeshRenderPass::Clear()
	{
		_items.clear();
		_renderBatchList.clear();
	}

	void StaticMeshRenderPass::BuildRenderBatches(const BoundingFrustum* worldFrustum)
	{
		std::vector<StaticMeshBatchItem> batchItems;

		for (const StaticMeshRenderItem& item : _items)
		{
			if (worldFrustum != nullptr && IsBoundingVolumeVisible(*worldFrustum, item.worldBounds) == false)
			{
#if GM_ENABLE_DEBUG_TOOLS
				++_lastCulledItemCount;
#endif
				continue;
			}

#if GM_ENABLE_DEBUG_TOOLS
			++_lastVisibleItemCount;
#endif

			const StaticMesh& staticMesh = *item.staticMesh;
			const std::shared_ptr<Mesh>& mesh = staticMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			for (const MeshSection& section : staticMesh.GetSections())
			{
				if (section.indexCount == 0 || section.textureSetIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.textureSetIndex];
				if (material == nullptr || material->GetVertexShader() == nullptr || material->GetPixelShader() == nullptr)
					continue;

				StaticMeshBatchItem batchItem{};
				batchItem.key.mesh = mesh.get();
				batchItem.key.material = material;
				batchItem.key.materialStateHash = material->GetRenderStateHash();
				batchItem.key.indexStart = section.indexStart;
				batchItem.key.indexCount = section.indexCount;
				batchItem.world = item.world;

				batchItems.push_back(std::move(batchItem));
			}
		}

		std::sort(batchItems.begin(), batchItems.end(), CompareBatchItems);
		for (const StaticMeshBatchItem& item : batchItems)
			AppendBatchItem(_renderBatchList, item);
	}

	void StaticMeshRenderPass::BindMaterialConstantData(const Material& material)
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
