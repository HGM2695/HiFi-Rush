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

	void SkeletalMeshComponent::OnTick(float deltaTime)
	{
		if (_skeletalMesh == nullptr)
			return;

		_animator.Tick(deltaTime, *_skeletalMesh, _pose);
	}

	void SkeletalMeshComponent::OnRender()
	{
		if (_skeletalMesh == nullptr || _ownerTransform == nullptr || _pose.IsValid() == false)
			return;

		SkeletalMeshRenderItem item{};
		item.world = _preTransform * _ownerTransform->GetWorldMatrix();
		item.skeletalMesh = _skeletalMesh.get();
		item.boneModelMatrices = &_pose.GetBoneModelMatrices();
		item.materials.reserve(_skeletalMesh->GetTextureSetCount());

		for (uint32 i = 0; i < _skeletalMesh->GetTextureSetCount(); ++i)
			item.materials.push_back(GetMaterial(i));

		APPLICATION.GetRenderer().SubmitSkeletalMesh(item);
	}

	void SkeletalMeshComponent::SetSkeletalMesh(const std::shared_ptr<SkeletalMesh>& skeletalMesh)
	{
		GM_ASSERT_RETURN(skeletalMesh, "SkeletalMeshComponent에 설정할 SkeletalMesh가 nullptr입니다.");

		_skeletalMesh = skeletalMesh;
		_preTransform = _skeletalMesh->GetPreTransform();
		_pose.RebuildBindPose(*_skeletalMesh);

		const uint32 slotCount = _skeletalMesh->GetTextureSetCount();
		_materials.clear();
		_materials.reserve(slotCount);

		Resources& resources = APPLICATION.GetResources();
		for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
		{
			const MeshTextureSet* textureSet = _skeletalMesh->GetTextureSet(slotIndex);
			if (textureSet == nullptr)
			{
				_materials.push_back(nullptr);
				continue;
			}

			Material::MaterialBuilder builder(resources);
			builder.SetVertexShader(BuiltinResourceKey::SkeletalMeshVS)
				.SetPixelShader(BuiltinResourceKey::StaticMeshPS)
				.SetSamplerAddressMode(TextureSlot::BaseColor, TextureAddressMode::Wrap);

			for (uint32 textureSetIndex = 0; textureSetIndex < TextureSlotCount; ++textureSetIndex)
			{
				const std::wstring& textureKey = textureSet->textureKeys[textureSetIndex];
				if (textureKey.empty())
					continue;

				builder.SetTexture(ToTextureSlot(textureSetIndex), textureKey);
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
