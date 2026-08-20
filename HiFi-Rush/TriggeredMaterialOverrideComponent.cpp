#include "TriggeredMaterialOverrideComponent.h"

#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "Material.h"
#include "Resources.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"
#include "Texture.h"

#include <utility>

namespace gm
{
	TriggeredMaterialOverrideComponent::TriggeredMaterialOverrideComponent(Resources& resources, const BeatSystem& beatSystem, TriggeredMaterialOverrideDesc desc)
		: _resources(resources), _beatSystem(beatSystem), _desc(std::move(desc))
	{}

	void TriggeredMaterialOverrideComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.overrides.empty() == false, "TriggeredMaterialOverrideComponent의 Material Override가 비어 있습니다.");
		GM_ASSERT_RETURN(ResolveOverrides(), "TriggeredMaterialOverrideComponent의 Material Override 구성에 실패했습니다.");

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "TriggeredMaterialOverrideComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { ScheduleOverride(startBeat); },
			[this]() { ResetOverrides(); }), "TriggeredMaterialOverrideComponent의 Trigger Binding에 실패했습니다.");
		ResetOverrides();
	}

	void TriggeredMaterialOverrideComponent::OnTick(float)
	{
		if (_isApplied || _triggerBeat.has_value() == false || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < *_triggerBeat)
			return;
		ApplyOverrides();
	}

	bool TriggeredMaterialOverrideComponent::ResolveOverrides()
	{
		const bool hasStaticMesh = GetOwner().GetComponent<StaticMeshComponent>() != nullptr;
		const bool hasSkeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>() != nullptr;
		GM_ASSERT_RETURN_VAL(hasStaticMesh != hasSkeletalMesh, false, "TriggeredMaterialOverrideComponent는 하나의 Static 또는 Skeletal Mesh Component가 필요합니다.");

		_overrides.clear();
		_overrides.reserve(_desc.overrides.size());
		for (const MaterialTextureOverrideDesc& overrideDesc : _desc.overrides)
		{
			Material* material = ResolveMaterial(overrideDesc.materialSlot);
			GM_ASSERT_RETURN_VAL(material, false, "Material Override 대상 Slot이 없습니다. slot=%u", overrideDesc.materialSlot);
			std::shared_ptr<Texture> texture = _resources.Find<Texture>(overrideDesc.textureKey);
			GM_ASSERT_RETURN_VAL(texture, false, "Material Override Texture가 없습니다. key=%ls", overrideDesc.textureKey.c_str());

			MaterialTextureOverride materialOverride{};
			materialOverride.material = material;
			materialOverride.textureSlot = overrideDesc.textureSlot;
			materialOverride.initialTexture = material->GetTexture(overrideDesc.textureSlot);
			materialOverride.overrideTexture = std::move(texture);
			_overrides.push_back(std::move(materialOverride));
		}
		return true;
	}

	Material* TriggeredMaterialOverrideComponent::ResolveMaterial(uint32 materialSlot)
	{
		if (StaticMeshComponent* staticMesh = GetOwner().GetComponent<StaticMeshComponent>())
			return staticMesh->GetMaterial(materialSlot);
		if (SkeletalMeshComponent* skeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>())
			return skeletalMesh->GetMaterial(materialSlot);
		return nullptr;
	}

	void TriggeredMaterialOverrideComponent::ScheduleOverride(float startBeat)
	{
		if (_isApplied || _triggerBeat.has_value())
			return;
		_triggerBeat = startBeat;
	}

	void TriggeredMaterialOverrideComponent::ResetOverrides()
	{
		for (const MaterialTextureOverride& materialOverride : _overrides)
			materialOverride.material->SetTexture(materialOverride.textureSlot, materialOverride.initialTexture);
		_triggerBeat.reset();
		_isApplied = false;
	}

	void TriggeredMaterialOverrideComponent::ApplyOverrides()
	{
		for (const MaterialTextureOverride& materialOverride : _overrides)
			materialOverride.material->SetTexture(materialOverride.textureSlot, materialOverride.overrideTexture);
		_triggerBeat.reset();
		_isApplied = true;
	}
}
