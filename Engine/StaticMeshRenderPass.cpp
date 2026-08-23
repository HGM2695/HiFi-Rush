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

		float CalculateCameraDepth(const StaticMeshRenderItem& item, const Matrix& view)
		{
			const Vector3 center = item.worldBounds.isValid ? Vector3{ item.worldBounds.box.Center.x, item.worldBounds.box.Center.y, item.worldBounds.box.Center.z } : Vector3::Transform(Vector3::Zero, item.world);
			return Vector3::Transform(center, view).z;
		}

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

		void BuildRenderBatches(std::vector<StaticMeshBatchItem>& items, std::vector<StaticMeshRenderBatch>& batches)
		{
			std::sort(items.begin(), items.end(), CompareBatchItems);
			for (const StaticMeshBatchItem& item : items)
				AppendBatchItem(batches, item);
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
		_gBufferPixelShader = _resources.Find<Shader>(BuiltinResourceKey::MeshGBufferPS);
		GM_ASSERT_RETURN_VAL(_gBufferPixelShader, false, "Mesh G-Buffer Pixel Shader를 찾지 못했습니다.");

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

	void StaticMeshRenderPass::Prepare(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum)
	{
		_constantBufferPool.ResetUsage();
		_opaqueRenderBatches.clear();
		_maskedRenderBatches.clear();
		_transparentRenderBatches.clear();
		_cameraBuffer = nullptr;

#if GM_ENABLE_DEBUG_TOOLS
		_lastSubmittedItemCount = static_cast<uint32>(_items.size());
		_lastVisibleItemCount = 0;
		_lastCulledItemCount = 0;
		_lastRenderBatchCount = 0;
		_lastNormalDrawCallCount = 0;
		_lastInstancedDrawCallCount = 0;
		_lastInstancedInstanceCount = 0;
#endif

		BuildRenderQueues(viewInfo.view, worldFrustum);

#if GM_ENABLE_DEBUG_TOOLS
		_lastRenderBatchCount = static_cast<uint32>(_opaqueRenderBatches.size() + _maskedRenderBatches.size() + _transparentRenderBatches.size());
#endif

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;

		_cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*_cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
	}

	void StaticMeshRenderPass::RenderOpaqueAndMasked(bool isInstancingEnabled)
	{
		BindCameraConstant();
		for (const StaticMeshRenderBatch& batch : _opaqueRenderBatches)
			RenderBatch(batch, isInstancingEnabled, true);
		for (const StaticMeshRenderBatch& batch : _maskedRenderBatches)
			RenderBatch(batch, isInstancingEnabled, true);
	}

	void StaticMeshRenderPass::AppendTransparentRenderEntries(std::vector<TransparentRenderEntry>& entries) const
	{
		for (uint32 itemIndex = 0; itemIndex < _transparentRenderBatches.size(); ++itemIndex)
		{
			const StaticMeshRenderBatch& batch = _transparentRenderBatches[itemIndex];
			entries.push_back(TransparentRenderEntry{ TransparentRenderSource::Static, itemIndex, batch.cameraDepth, batch.submissionOrder });
		}
	}

	void StaticMeshRenderPass::RenderTransparent(uint32 itemIndex)
	{
		GM_ASSERT_RETURN(itemIndex < _transparentRenderBatches.size(), "Static Mesh Transparent Render Item Index가 범위를 벗어났습니다.");
		BindCameraConstant();
		RenderBatch(_transparentRenderBatches[itemIndex], false, false);
	}

	void StaticMeshRenderPass::Clear()
	{
		_items.clear();
		_opaqueRenderBatches.clear();
		_maskedRenderBatches.clear();
		_transparentRenderBatches.clear();
		_cameraBuffer = nullptr;
	}

	void StaticMeshRenderPass::BuildRenderQueues(const Matrix& view, const BoundingFrustum* worldFrustum)
	{
		std::vector<StaticMeshBatchItem> opaqueItems;
		std::vector<StaticMeshBatchItem> maskedItems;

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
			const float cameraDepth = CalculateCameraDepth(item, view);

			const StaticMesh& staticMesh = *item.staticMesh;
			const std::shared_ptr<Mesh>& mesh = staticMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			for (const MeshSection& section : staticMesh.GetSections())
			{
				if (section.indexCount == 0 || section.materialSlotIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.materialSlotIndex];
				if (material == nullptr || material->GetVertexShader() == nullptr || material->GetPixelShader() == nullptr)
					continue;

				StaticMeshBatchItem batchItem{};
				batchItem.key.mesh = mesh.get();
				batchItem.key.material = material;
				batchItem.key.materialStateHash = material->GetRenderStateHash();
				batchItem.key.indexStart = section.indexStart;
				batchItem.key.indexCount = section.indexCount;
				batchItem.world = item.world;

				switch (material->GetSurfaceMode())
				{
				case SurfaceMode::Opaque:
					opaqueItems.push_back(std::move(batchItem));
					break;
				case SurfaceMode::Masked:
					maskedItems.push_back(std::move(batchItem));
					break;
				case SurfaceMode::Transparent:
				{
					StaticMeshRenderBatch batch{};
					batch.key = batchItem.key;
					batch.worlds.push_back(batchItem.world);
					batch.cameraDepth = cameraDepth;
					batch.submissionOrder = item.submissionOrder;
					_transparentRenderBatches.push_back(std::move(batch));
					break;
				}
				default:
					break;
				}
			}
		}

		BuildRenderBatches(opaqueItems, _opaqueRenderBatches);
		BuildRenderBatches(maskedItems, _maskedRenderBatches);
	}

	void StaticMeshRenderPass::BindCameraConstant()
	{
		GM_ASSERT_RETURN(_cameraBuffer, "Static Mesh Camera ConstantBuffer가 준비되지 않았습니다.");
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, _cameraBuffer);
	}

	void StaticMeshRenderPass::RenderBatch(const StaticMeshRenderBatch& batch, bool isInstancingEnabled, bool isGBufferPass)
	{
		const StaticMeshBatchKey& key = batch.key;
		if (key.mesh == nullptr || key.material == nullptr || key.indexCount == 0)
			return;

		_commandContext.BindMesh(*key.mesh);
		_commandContext.BindMaterial(*key.material);
		if (isGBufferPass)
		{
			_commandContext.BindVertexShader(*_staticMeshVertexShader);
			_commandContext.BindPixelShader(*_gBufferPixelShader);
		}
		BindMaterialConstantData(*key.material);
		BindMaterialSurfaceConstant(*key.material);
		if (CanRenderInstanced(batch, isInstancingEnabled) && RenderInstancedBatch(batch))
			return;

		RenderNormalBatch(batch);
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

	void StaticMeshRenderPass::BindMaterialSurfaceConstant(const Material& material)
	{
		const MaterialSurfaceData& surfaceData = material.GetSurfaceData();
		MaterialSurfaceConstantPS constant{};
		constant.shadingModel = static_cast<uint32>(surfaceData.shadingModel);
		constant.surfaceMode = static_cast<uint32>(surfaceData.surfaceMode);
		constant.outlineMode = static_cast<uint32>(surfaceData.outlineMode);
		constant.emissiveIntensity = surfaceData.emissiveIntensity;
		constant.alphaCutoff = surfaceData.alphaCutoff;
		constant.emissiveColor = ConvertSRGBToLinear(surfaceData.emissiveColor);
		const MaterialColorData& colorData = material.GetColorData();
		constant.colorMode = static_cast<uint32>(colorData.mode);
		for (uint32 textureSlotIndex = 0; textureSlotIndex < TextureSlotCount; ++textureSlotIndex)
		{
			if (material.GetTexture(ToTextureSlot(textureSlotIndex)))
				constant.textureFlags |= 1u << textureSlotIndex;
		}
		constant.colorBlendRatio = colorData.blendRatio;
		constant.blendColor = ConvertSRGBToLinear(colorData.blendColor);
		constant.opacityLowColor = ConvertSRGBToLinear(colorData.opacityLowColor);
		constant.opacityHighColor = ConvertSRGBToLinear(colorData.opacityHighColor);
		constant.colorMultiplier = colorData.colorMultiplier;
		constant.textureUVOffset = material.GetTextureUVOffset();
		ConstantBuffer* buffer = _constantBufferPool.Acquire(sizeof(MaterialSurfaceConstantPS));
		_commandContext.UpdateConstantBuffer(*buffer, &constant, sizeof(MaterialSurfaceConstantPS));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, buffer);
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
