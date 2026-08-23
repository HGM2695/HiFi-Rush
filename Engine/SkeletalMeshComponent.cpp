#include "SkeletalMeshComponent.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "GameObject.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "SkeletalMesh.h"
#include "TransformComponent.h"

namespace gm
{
	SkeletalMeshComponent::SkeletalMeshComponent() = default;
	SkeletalMeshComponent::~SkeletalMeshComponent() = default;

	void SkeletalMeshComponent::OnRender()
	{
		if (_skeletalMesh == nullptr || _ownerTransform == nullptr || _pose.IsValid() == false)
			return;

		SkeletalMeshRenderItem item{};
		item.world = _preTransform * _ownerTransform->GetWorldMatrix();
		item.worldBounds = TransformBoundingVolume(_skeletalMesh->GetLocalBounds(), item.world);
		item.skeletalMesh = _skeletalMesh.get();
		item.boneModelMatrices = &_pose.GetBoneModelMatrices();
		item.castsShadow = _castsShadow;
		item.materials.reserve(_skeletalMesh->GetMaterialSlotCount());

		for (uint32 i = 0; i < _skeletalMesh->GetMaterialSlotCount(); ++i)
			item.materials.push_back(GetMaterial(i));

		APPLICATION.GetRenderer().SubmitSkeletalMesh(item);
	}

	BoundingVolume SkeletalMeshComponent::GetLocalBounds() const
	{
		if (_skeletalMesh == nullptr)
			return BoundingVolume{};

		return _skeletalMesh->GetLocalBounds();
	}

	BoundingVolume SkeletalMeshComponent::GetWorldBounds() const
	{
		if (_skeletalMesh == nullptr || _ownerTransform == nullptr)
			return BoundingVolume{};

		const Matrix world = _preTransform * _ownerTransform->GetWorldMatrix();
		return TransformBoundingVolume(_skeletalMesh->GetLocalBounds(), world);
	}

	void SkeletalMeshComponent::SetSkeletalMesh(const std::shared_ptr<SkeletalMesh>& skeletalMesh)
	{
		GM_ASSERT_RETURN(skeletalMesh, "SkeletalMeshComponent에 설정할 SkeletalMesh가 nullptr입니다.");

		_skeletalMesh = skeletalMesh;
		_preTransform = _skeletalMesh->GetPreTransform();
		_castsShadow = _skeletalMesh->CastsShadow();
		_pose.RebuildBindPose(*_skeletalMesh);

		const uint32 slotCount = _skeletalMesh->GetMaterialSlotCount();
		_materials.clear();
		_materials.reserve(slotCount);

		Resources& resources = APPLICATION.GetResources();
		for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
		{
			const MeshMaterialSlot* materialSlot = _skeletalMesh->GetMaterialSlot(slotIndex);
			if (materialSlot == nullptr)
			{
				_materials.push_back(nullptr);
				continue;
			}

			Material::MaterialBuilder builder(resources);
			builder.SetSurfaceData(materialSlot->surfaceData)
				.SetCullMode(materialSlot->cullMode)
				.SetVertexShader(BuiltinResourceKey::SkeletalMeshVS)
				.SetPixelShader(BuiltinResourceKey::MeshForwardPS)
				.SetSamplerAddressMode(TextureSlot::BaseColor, materialSlot->baseColorAddressMode);

			for (uint32 textureSlotIndex = 0; textureSlotIndex < TextureSlotCount; ++textureSlotIndex)
			{
				const std::wstring& textureKey = materialSlot->textureKeys[textureSlotIndex];
				if (textureKey.empty())
					continue;

				builder.SetTexture(ToTextureSlot(textureSlotIndex), textureKey);
			}

			_materials.push_back(std::make_unique<Material>(builder.Build()));
		}
	}

	void SkeletalMeshComponent::SetMaterial(uint32 slotIndex, const Material& material)
	{
		if (slotIndex >= _materials.size())
			_materials.resize(slotIndex + 1);

		_materials[slotIndex] = std::make_unique<Material>(material);
	}

	Material* SkeletalMeshComponent::GetMaterial(uint32 slotIndex) const
	{
		if (slotIndex >= _materials.size())
			return nullptr;

		return _materials[slotIndex].get();
	}

	void SkeletalMeshComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<TransformComponent>();
		GM_ASSERT_RETURN(_ownerTransform, "SkeletalMeshComponent 소유자의 TransformComponent가 존재하지 않습니다.");
	}
}
