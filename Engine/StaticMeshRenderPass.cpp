#include "StaticMeshRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "InstanceBuffer.h"
#include "Material.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "StaticMesh.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

#include <algorithm>
#include <functional>
#include <limits>

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
		_staticMeshVertexShader = _resources.Find<Shader>(BuiltinResourceKey::StaticMeshVS);
		GM_ASSERT_RETURN_VAL(_staticMeshVertexShader, false, "Static Mesh Vertex Shader를 찾지 못했습니다.");

		_staticMeshInstancedVertexShader = _resources.Find<Shader>(BuiltinResourceKey::StaticMeshInstancedVS);
		GM_ASSERT_RETURN_VAL(_staticMeshInstancedVertexShader, false, "Static Mesh Instanced Vertex Shader를 찾지 못했습니다.");

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

	void StaticMeshRenderPass::Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum, bool isInstancingEnabled)
	{
		_constantBufferPool.ResetUsage();
		_renderBatchList.clear();

#if GM_ENABLE_DEBUG_TOOLS
		_lastSubmittedItemCount = static_cast<uint32>(_items.size());
		_lastVisibleItemCount = 0;
		_lastCulledItemCount = 0;
		_lastRenderBatchCount = 0;
		_lastNormalDrawCallCount = 0;
		_lastInstancedDrawCallCount = 0;
		_lastInstancedInstanceCount = 0;
#endif

		BuildRenderBatches(worldFrustum);

#if GM_ENABLE_DEBUG_TOOLS
		_lastRenderBatchCount = static_cast<uint32>(_renderBatchList.size());
#endif

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

			if (CanRenderInstanced(batch, isInstancingEnabled) && RenderInstancedBatch(batch))
				continue;

			RenderNormalBatch(batch);
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

	bool StaticMeshRenderPass::CanRenderInstanced(const StaticMeshRenderBatch& batch, bool isInstancingEnabled) const
	{
		if (isInstancingEnabled == false || batch.worlds.size() <= 1 || batch.key.material == nullptr)
			return false;

		if (batch.worlds.size() > std::numeric_limits<uint32>::max())
			return false;

		return batch.key.material->GetVertexShader() == _staticMeshVertexShader && _staticMeshInstancedVertexShader != nullptr;
	}

	bool StaticMeshRenderPass::EnsureInstanceBufferCapacity(uint32 requiredCapacity)
	{
		if (_instanceBuffer != nullptr && _instanceBuffer->GetCapacity() >= requiredCapacity)
			return true;

		const uint32 currentCapacity = _instanceBuffer != nullptr ? _instanceBuffer->GetCapacity() : 0;
		const uint32 grownCapacity = currentCapacity > 0 && currentCapacity <= std::numeric_limits<uint32>::max() / 2
			? currentCapacity * 2 : currentCapacity;
		const uint32 newCapacity = std::max({ requiredCapacity, grownCapacity, 256u });

		InstanceBufferDesc desc{};
		desc.stride = sizeof(Matrix);
		desc.capacity = newCapacity;

		std::unique_ptr<InstanceBuffer> instanceBuffer = _resourceFactory.CreateInstanceBuffer(desc);
		GM_ASSERT_RETURN_VAL(instanceBuffer, false, "Static Mesh Instance Buffer 생성에 실패했습니다.");

		_instanceBuffer = std::move(instanceBuffer);
		return true;
	}

	void StaticMeshRenderPass::RenderNormalBatch(const StaticMeshRenderBatch& batch)
	{
		for (const Matrix& world : batch.worlds)
		{
			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = world;

			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
			_commandContext.DrawIndexed(batch.key.indexCount, batch.key.indexStart, 0);

#if GM_ENABLE_DEBUG_TOOLS
			++_lastNormalDrawCallCount;
#endif
		}
	}

	bool StaticMeshRenderPass::RenderInstancedBatch(const StaticMeshRenderBatch& batch)
	{
		const uint32 instanceCount = static_cast<uint32>(batch.worlds.size());
		if (EnsureInstanceBufferCapacity(instanceCount) == false)
			return false;

		if (_commandContext.UpdateInstanceBuffer(*_instanceBuffer, batch.worlds.data(), instanceCount) == false)
			return false;

		_commandContext.BindVertexShader(*_staticMeshInstancedVertexShader);
		_commandContext.BindInstanceBuffer(*_instanceBuffer);
		_commandContext.DrawIndexedInstanced(batch.key.indexCount, instanceCount, batch.key.indexStart, 0);

#if GM_ENABLE_DEBUG_TOOLS
		++_lastInstancedDrawCallCount;
		_lastInstancedInstanceCount += instanceCount;
#endif

		return true;
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
