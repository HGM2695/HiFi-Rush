#include "ShadowCasterPass.h"
#include "BuiltinGraphicsResources.h"
#include "CascadedShadowMap.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "InstanceBuffer.h"
#include "Material.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "ShadowTypes.h"
#include "SkeletalMesh.h"
#include "SkinningRenderData.h"
#include "StaticMesh.h"
#include "Texture.h"
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

		struct ShadowViewConstantVS
		{
			Matrix lightViewProjection;
		};

		struct ShadowMaterialConstantPS
		{
			float alphaCutoff = 0.5f;
			Vector3 padding{};
			Vector2 textureUVOffset{};
			Vector2 textureUVPadding{};
		};

		struct StaticShadowBatchItem
		{
			StaticMeshBatchKey key{};
			Matrix world = Matrix::CreateScale(1.f);
		};

		bool CompareStaticBatchItems(const StaticShadowBatchItem& lhs, const StaticShadowBatchItem& rhs)
		{
			if (lhs.key.materialStateHash != rhs.key.materialStateHash)
				return lhs.key.materialStateHash < rhs.key.materialStateHash;
			if (lhs.key.mesh != rhs.key.mesh)
				return std::less<const Mesh*>{}(lhs.key.mesh, rhs.key.mesh);
			if (lhs.key.indexStart != rhs.key.indexStart)
				return lhs.key.indexStart < rhs.key.indexStart;
			return lhs.key.indexCount < rhs.key.indexCount;
		}

		void BuildStaticBatches(std::vector<StaticShadowBatchItem>& items, std::vector<StaticMeshRenderBatch>& batches)
		{
			std::sort(items.begin(), items.end(), CompareStaticBatchItems);
			for (const StaticShadowBatchItem& item : items)
			{
				if (batches.empty() == false && batches.back().key == item.key)
				{
					batches.back().worlds.push_back(item.world);
					continue;
				}
				StaticMeshRenderBatch batch{};
				batch.key = item.key;
				batch.worlds.push_back(item.world);
				batches.push_back(std::move(batch));
			}
		}
	}

	ShadowCasterPass::ShadowCasterPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	ShadowCasterPass::~ShadowCasterPass() = default;

	bool ShadowCasterPass::Initialize()
	{
		_staticVertexShader = _resources.Find<Shader>(BuiltinResourceKey::StaticShadowVS);
		_staticInstancedVertexShader = _resources.Find<Shader>(BuiltinResourceKey::StaticInstancedShadowVS);
		_skeletalVertexShader = _resources.Find<Shader>(BuiltinResourceKey::SkeletalShadowVS);
		_maskedPixelShader = _resources.Find<Shader>(BuiltinResourceKey::MaskedShadowPS);
		GM_ASSERT_RETURN_VAL(_staticVertexShader && _staticInstancedVertexShader && _skeletalVertexShader && _maskedPixelShader, false, "Shadow Caster Shader를 찾지 못했습니다.");
		return true;
	}

	void ShadowCasterPass::Submit(const StaticMeshRenderItem& item)
	{
		if (item.staticMesh)
			_staticItems.push_back(item);
	}

	void ShadowCasterPass::Submit(const SkeletalMeshRenderItem& item)
	{
		if (item.skeletalMesh && item.boneModelMatrices)
			_skeletalItems.push_back(item);
	}

	void ShadowCasterPass::RenderCascade(CascadedShadowMap& shadowMap, uint32 cascadeIndex, const Matrix& lightViewProjection, const BoundingOrientedBox* casterBounds, bool isInstancingEnabled)
	{
		GM_ASSERT_RETURN(cascadeIndex < MaxShadowCascadeCount, "Shadow Cascade Index가 범위를 벗어났습니다.");
		_constantBufferPool.ResetUsage();
		BuildRenderQueues(casterBounds);

		Texture& shadowTexture = shadowMap.GetTexture();
		_commandContext.ClearDepthStencilSlice(shadowTexture, cascadeIndex);
		_commandContext.BindDepthStencilSlice(shadowTexture, cascadeIndex);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(shadowTexture.GetWidth()), static_cast<float>(shadowTexture.GetHeight()) });
		DepthStencilDesc depthStencilDesc{};
		depthStencilDesc.depthEnable = true;
		depthStencilDesc.depthWriteEnable = true;
		depthStencilDesc.depthFunc = CompareFunc::LessEqual;
		_commandContext.BindDepthStencilState(depthStencilDesc);
		_commandContext.BindBlendState(BlendDesc{});

		ShadowViewConstantVS shadowView{};
		shadowView.lightViewProjection = lightViewProjection;
		ConstantBuffer* shadowViewBuffer = _constantBufferPool.Acquire(sizeof(ShadowViewConstantVS));
		_commandContext.UpdateConstantBuffer(*shadowViewBuffer, &shadowView, sizeof(ShadowViewConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, shadowViewBuffer);

		for (const StaticMeshRenderBatch& batch : _opaqueStaticBatches)
			RenderStaticBatch(batch, false, isInstancingEnabled);
		for (const StaticMeshRenderBatch& batch : _maskedStaticBatches)
			RenderStaticBatch(batch, true, isInstancingEnabled);
		for (const SkeletalShadowRenderItem& item : _opaqueSkeletalItems)
			RenderSkeletal(item, false);
		for (const SkeletalShadowRenderItem& item : _maskedSkeletalItems)
			RenderSkeletal(item, true);
		_commandContext.UnbindShaderTextures(0, 1);
	}

	void ShadowCasterPass::Clear()
	{
		_staticItems.clear();
		_skeletalItems.clear();
		_opaqueStaticBatches.clear();
		_maskedStaticBatches.clear();
		_opaqueSkeletalItems.clear();
		_maskedSkeletalItems.clear();
	}

	void ShadowCasterPass::BuildRenderQueues(const BoundingOrientedBox* casterBounds)
	{
		_opaqueStaticBatches.clear();
		_maskedStaticBatches.clear();
		_opaqueSkeletalItems.clear();
		_maskedSkeletalItems.clear();
		std::vector<StaticShadowBatchItem> opaqueStaticItems;
		std::vector<StaticShadowBatchItem> maskedStaticItems;

		for (const StaticMeshRenderItem& item : _staticItems)
		{
			if (casterBounds && item.worldBounds.isValid && casterBounds->Intersects(item.worldBounds.box) == false)
				continue;

			const StaticMesh& staticMesh = *item.staticMesh;
			const std::shared_ptr<Mesh>& mesh = staticMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			for (const MeshSection& section : staticMesh.GetSections())
			{
				if (section.indexCount == 0 || section.materialSlotIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.materialSlotIndex];
				if (material == nullptr || material->GetSurfaceMode() == SurfaceMode::Transparent)
					continue;

				StaticShadowBatchItem batchItem{};
				batchItem.key.mesh = mesh.get();
				batchItem.key.material = material;
				batchItem.key.materialStateHash = material->GetRenderStateHash();
				batchItem.key.indexStart = section.indexStart;
				batchItem.key.indexCount = section.indexCount;
				batchItem.world = item.world;
				(material->GetSurfaceMode() == SurfaceMode::Masked ? maskedStaticItems : opaqueStaticItems).push_back(std::move(batchItem));
			}
		}
		BuildStaticBatches(opaqueStaticItems, _opaqueStaticBatches);
		BuildStaticBatches(maskedStaticItems, _maskedStaticBatches);

		for (const SkeletalMeshRenderItem& item : _skeletalItems)
		{
			if (casterBounds && item.worldBounds.isValid && casterBounds->Intersects(item.worldBounds.box) == false)
				continue;
			const SkeletalMesh& skeletalMesh = *item.skeletalMesh;
			const std::shared_ptr<Mesh>& mesh = skeletalMesh.GetMesh();
			if (mesh == nullptr)
				continue;
			const std::vector<MeshSection>& sections = skeletalMesh.GetSections();
			for (uint32 sectionIndex = 0; sectionIndex < sections.size(); ++sectionIndex)
			{
				const MeshSection& section = sections[sectionIndex];
				if (section.indexCount == 0 || section.materialSlotIndex >= item.materials.size())
					continue;
				const Material* material = item.materials[section.materialSlotIndex];
				if (material == nullptr || material->GetSurfaceMode() == SurfaceMode::Transparent)
					continue;
				SkeletalShadowRenderItem renderItem{};
				renderItem.world = item.world;
				renderItem.mesh = mesh.get();
				renderItem.section = &section;
				renderItem.boneModelMatrices = item.boneModelMatrices;
				renderItem.material = material;
				renderItem.materialStateHash = material->GetRenderStateHash();
				(material->GetSurfaceMode() == SurfaceMode::Masked ? _maskedSkeletalItems : _opaqueSkeletalItems).push_back(std::move(renderItem));
			}
		}

		auto compareSkeletalItems = [](const SkeletalShadowRenderItem& lhs, const SkeletalShadowRenderItem& rhs)
		{
			if (lhs.materialStateHash != rhs.materialStateHash)
				return lhs.materialStateHash < rhs.materialStateHash;
			if (lhs.mesh != rhs.mesh)
				return std::less<const Mesh*>{}(lhs.mesh, rhs.mesh);
			return lhs.section->indexStart < rhs.section->indexStart;
		};
		std::sort(_opaqueSkeletalItems.begin(), _opaqueSkeletalItems.end(), compareSkeletalItems);
		std::sort(_maskedSkeletalItems.begin(), _maskedSkeletalItems.end(), compareSkeletalItems);
	}

	void ShadowCasterPass::RenderStaticBatch(const StaticMeshRenderBatch& batch, bool isMasked, bool isInstancingEnabled)
	{
		const StaticMeshBatchKey& key = batch.key;
		_commandContext.BindMesh(*key.mesh);
		BindShadowMaterial(*key.material, isMasked);
		if (isInstancingEnabled && batch.worlds.size() > 1 && batch.worlds.size() <= std::numeric_limits<uint32>::max() && RenderStaticInstanced(batch))
			return;
		_commandContext.BindVertexShader(*_staticVertexShader);
		RenderStaticNormal(batch);
	}

	void ShadowCasterPass::RenderStaticNormal(const StaticMeshRenderBatch& batch)
	{
		for (const Matrix& world : batch.worlds)
		{
			ObjectConstantVS object{};
			object.world = world;
			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*objectBuffer, &object, sizeof(ObjectConstantVS));
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
			_commandContext.DrawIndexed(batch.key.indexCount, batch.key.indexStart, 0);
		}
	}

	bool ShadowCasterPass::RenderStaticInstanced(const StaticMeshRenderBatch& batch)
	{
		const uint32 instanceCount = static_cast<uint32>(batch.worlds.size());
		if (EnsureInstanceBufferCapacity(instanceCount) == false || _commandContext.UpdateInstanceBuffer(*_instanceBuffer, batch.worlds.data(), instanceCount) == false)
			return false;
		_commandContext.BindVertexShader(*_staticInstancedVertexShader);
		_commandContext.BindInstanceBuffer(*_instanceBuffer);
		_commandContext.DrawIndexedInstanced(batch.key.indexCount, instanceCount, batch.key.indexStart, 0);
		return true;
	}

	void ShadowCasterPass::RenderSkeletal(const SkeletalShadowRenderItem& item, bool isMasked)
	{
		ObjectConstantVS object{};
		object.world = item.world;
		ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
		_commandContext.UpdateConstantBuffer(*objectBuffer, &object, sizeof(ObjectConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
		BonePaletteConstantVS bonePalette = BuildBonePalette(*item.section, *item.boneModelMatrices);
		ConstantBuffer* boneBuffer = _constantBufferPool.Acquire(sizeof(BonePaletteConstantVS));
		_commandContext.UpdateConstantBuffer(*boneBuffer, &bonePalette, sizeof(BonePaletteConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 2, boneBuffer);
		_commandContext.BindMesh(*item.mesh);
		_commandContext.BindVertexShader(*_skeletalVertexShader);
		BindShadowMaterial(*item.material, isMasked);
		_commandContext.DrawIndexed(item.section->indexCount, item.section->indexStart, 0);
	}

	void ShadowCasterPass::BindShadowMaterial(const Material& material, bool isMasked)
	{
		_commandContext.BindPrimitiveTopology(material.GetTopology());
		_commandContext.BindRasterizerState(material.GetRasterizerDesc());
		if (isMasked == false)
		{
			_commandContext.UnbindPixelShader();
			return;
		}
		_commandContext.BindPixelShader(*_maskedPixelShader);
		_commandContext.BindShaderTexture(0, material.GetTexture(TextureSlot::BaseColor).get());
		_commandContext.BindSampler(0, &material.GetSamplerDesc(TextureSlot::BaseColor));
		ShadowMaterialConstantPS shadowMaterial{};
		shadowMaterial.alphaCutoff = material.GetAlphaCutoff();
		shadowMaterial.textureUVOffset = material.GetTextureUVOffset();
		ConstantBuffer* materialBuffer = _constantBufferPool.Acquire(sizeof(ShadowMaterialConstantPS));
		_commandContext.UpdateConstantBuffer(*materialBuffer, &shadowMaterial, sizeof(ShadowMaterialConstantPS));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, materialBuffer);
	}

	bool ShadowCasterPass::EnsureInstanceBufferCapacity(uint32 requiredCapacity)
	{
		if (_instanceBuffer && _instanceBuffer->GetCapacity() >= requiredCapacity)
			return true;
		const uint32 currentCapacity = _instanceBuffer ? _instanceBuffer->GetCapacity() : 0;
		const uint32 grownCapacity = currentCapacity > 0 && currentCapacity <= std::numeric_limits<uint32>::max() / 2 ? currentCapacity * 2 : currentCapacity;
		InstanceBufferDesc desc{};
		desc.stride = sizeof(Matrix);
		desc.capacity = std::max({ requiredCapacity, grownCapacity, 256u });
		_instanceBuffer = _resourceFactory.CreateInstanceBuffer(desc);
		return _instanceBuffer != nullptr;
	}
}
