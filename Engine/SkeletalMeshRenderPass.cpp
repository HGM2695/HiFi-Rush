#include "SkeletalMeshRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "Mesh.h"
#include "Resources.h"
#include "Shader.h"
#include "SkeletalMesh.h"
#include "SkinningRenderData.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

#include <algorithm>
#include <array>
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

		float CalculateCameraDepth(const SkeletalMeshRenderItem& item, const Matrix& view)
		{
			const Vector3 center = item.worldBounds.isValid ? Vector3{ item.worldBounds.box.Center.x, item.worldBounds.box.Center.y, item.worldBounds.box.Center.z } : Vector3::Transform(Vector3::Zero, item.world);
			return Vector3::Transform(center, view).z;
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
		_skeletalMeshVertexShader = _resources.Find<Shader>(BuiltinResourceKey::SkeletalMeshVS);
		GM_ASSERT_RETURN_VAL(_skeletalMeshVertexShader, false, "Skeletal Mesh Vertex Shader를 찾지 못했습니다.");
		_gBufferPixelShader = _resources.Find<Shader>(BuiltinResourceKey::MeshGBufferPS);
		GM_ASSERT_RETURN_VAL(_gBufferPixelShader, false, "Mesh G-Buffer Pixel Shader를 찾지 못했습니다.");
		return true;
	}

	void SkeletalMeshRenderPass::Submit(const SkeletalMeshRenderItem& item)
	{
		if (item.skeletalMesh == nullptr || item.boneModelMatrices == nullptr)
			return;

		_items.push_back(item);
	}

#if GM_ENABLE_DEBUG_TOOLS
	void SkeletalMeshRenderPass::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		for (const SkeletalMeshRenderItem& item : _items)
		{
			if (item.worldBounds.isValid)
				debugRenderer.RequestDrawBox(item.worldBounds.box, Colors::Cyan);
		}
	}
#endif

	void SkeletalMeshRenderPass::Prepare(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum)
	{
		_constantBufferPool.ResetUsage();
		_opaqueRenderItems.clear();
		_maskedRenderItems.clear();
		_transparentRenderItems.clear();
		_cameraBuffer = nullptr;

#if GM_ENABLE_DEBUG_TOOLS
		_lastSubmittedItemCount = static_cast<uint32>(_items.size());
		_lastVisibleItemCount = 0;
		_lastCulledItemCount = 0;
#endif
		BuildRenderQueues(viewInfo.view, worldFrustum);

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;

		_cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*_cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
	}

	void SkeletalMeshRenderPass::RenderOpaqueAndMasked()
	{
		BindCameraConstant();
		for (const SectionRenderItem& item : _opaqueRenderItems)
			RenderSection(item, true);
		for (const SectionRenderItem& item : _maskedRenderItems)
			RenderSection(item, true);
	}

	void SkeletalMeshRenderPass::AppendTransparentRenderEntries(std::vector<TransparentRenderEntry>& entries) const
	{
		for (uint32 itemIndex = 0; itemIndex < _transparentRenderItems.size(); ++itemIndex)
		{
			const SectionRenderItem& item = _transparentRenderItems[itemIndex];
			entries.push_back(TransparentRenderEntry{ TransparentRenderSource::Skeletal, itemIndex, item.cameraDepth, item.submissionOrder });
		}
	}

	void SkeletalMeshRenderPass::RenderTransparent(uint32 itemIndex)
	{
		GM_ASSERT_RETURN(itemIndex < _transparentRenderItems.size(), "Skeletal Mesh Transparent Render Item Index가 범위를 벗어났습니다.");
		BindCameraConstant();
		RenderSection(_transparentRenderItems[itemIndex], false);
	}

	void SkeletalMeshRenderPass::BuildRenderQueues(const Matrix& view, const BoundingFrustum* worldFrustum)
	{
		for (const SkeletalMeshRenderItem& item : _items)
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

			const SkeletalMesh& skeletalMesh = *item.skeletalMesh;
			const std::shared_ptr<Mesh>& mesh = skeletalMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			const std::vector<MeshSection>& sections = skeletalMesh.GetSections();
			for (uint32 sectionIndex = 0; sectionIndex < sections.size(); ++sectionIndex)
			{
				const MeshSection& section = sections[sectionIndex];
				if (section.indexCount == 0)
					continue;

				if (section.materialSlotIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.materialSlotIndex];
				if (material == nullptr || material->GetVertexShader() == nullptr || material->GetPixelShader() == nullptr)
					continue;

				SectionRenderItem renderItem{};
				renderItem.world = item.world;
				renderItem.mesh = mesh.get();
				renderItem.section = &section;
				renderItem.boneModelMatrices = item.boneModelMatrices;
				renderItem.material = material;
				renderItem.materialStateHash = material->GetRenderStateHash();
				renderItem.cameraDepth = cameraDepth;
				renderItem.submissionOrder = item.submissionOrder;

				switch (material->GetSurfaceMode())
				{
				case SurfaceMode::Opaque:
					_opaqueRenderItems.push_back(std::move(renderItem));
					break;
				case SurfaceMode::Masked:
					_maskedRenderItems.push_back(std::move(renderItem));
					break;
				case SurfaceMode::Transparent:
					_transparentRenderItems.push_back(std::move(renderItem));
					break;
				default:
					break;
				}
			}
		}

		auto compareRenderItems = [](const SectionRenderItem& lhs, const SectionRenderItem& rhs)
		{
			if (lhs.materialStateHash != rhs.materialStateHash)
				return lhs.materialStateHash < rhs.materialStateHash;
			if (lhs.mesh != rhs.mesh)
				return std::less<const Mesh*>{}(lhs.mesh, rhs.mesh);
			return lhs.section->indexStart < rhs.section->indexStart;
		};
		std::sort(_opaqueRenderItems.begin(), _opaqueRenderItems.end(), compareRenderItems);
		std::sort(_maskedRenderItems.begin(), _maskedRenderItems.end(), compareRenderItems);
	}

	void SkeletalMeshRenderPass::Clear()
	{
		_items.clear();
		_opaqueRenderItems.clear();
		_maskedRenderItems.clear();
		_transparentRenderItems.clear();
		_cameraBuffer = nullptr;
	}

	void SkeletalMeshRenderPass::BindCameraConstant()
	{
		GM_ASSERT_RETURN(_cameraBuffer, "Skeletal Mesh Camera ConstantBuffer가 준비되지 않았습니다.");
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, _cameraBuffer);
	}

	void SkeletalMeshRenderPass::RenderSection(const SectionRenderItem& item, bool isGBufferPass)
	{
		if (item.mesh == nullptr || item.section == nullptr || item.boneModelMatrices == nullptr || item.material == nullptr)
			return;

		ObjectConstantVS objectConstantVS{};
		objectConstantVS.world = item.world;
		ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
		_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
		_commandContext.BindMesh(*item.mesh);

		BonePaletteConstantVS bonePalette = BuildBonePalette(*item.section, *item.boneModelMatrices);
		ConstantBuffer* boneBuffer = _constantBufferPool.Acquire(sizeof(BonePaletteConstantVS));
		_commandContext.UpdateConstantBuffer(*boneBuffer, &bonePalette, sizeof(BonePaletteConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 2, boneBuffer);
		_commandContext.BindMaterial(*item.material);
		if (isGBufferPass)
		{
			_commandContext.BindVertexShader(*_skeletalMeshVertexShader);
			_commandContext.BindPixelShader(*_gBufferPixelShader);
		}
		BindMaterialConstantData(*item.material);
		BindMaterialSurfaceConstant(*item.material);
		_commandContext.DrawIndexed(item.section->indexCount, item.section->indexStart, 0);
	}

	void SkeletalMeshRenderPass::BindMaterialSurfaceConstant(const Material& material)
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
