#include "BeatTextureUVStepComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "Material.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"

#include <cmath>

namespace gm
{
	BeatTextureUVStepComponent::BeatTextureUVStepComponent(const BeatSystem& beatSystem, const BeatTextureUVStepDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatTextureUVStepComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.stepDurationBeats > 0.f, "BeatTextureUVStepComponent의 Step Duration은 0보다 커야 합니다.");
		GM_ASSERT_RETURN(ResolveMaterial(), "BeatTextureUVStepComponent의 Material을 찾을 수 없습니다. slot=%u", _desc.materialSlot);
		UpdateTextureUVOffset();
	}

	void BeatTextureUVStepComponent::OnTick(float)
	{
		UpdateTextureUVOffset();
	}

	bool BeatTextureUVStepComponent::ResolveMaterial()
	{
		StaticMeshComponent* staticMesh = GetOwner().GetComponent<StaticMeshComponent>();
		SkeletalMeshComponent* skeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL((staticMesh != nullptr) != (skeletalMesh != nullptr), false, "BeatTextureUVStepComponent는 하나의 Static 또는 Skeletal Mesh Component가 필요합니다.");
		_material = staticMesh ? staticMesh->GetMaterial(_desc.materialSlot) : skeletalMesh->GetMaterial(_desc.materialSlot);
		return _material != nullptr;
	}

	void BeatTextureUVStepComponent::UpdateTextureUVOffset()
	{
		if (_material == nullptr)
			return;

		const float currentBeat = _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() : 0.f;
		const uint64 stepIndex = static_cast<uint64>(std::floor(currentBeat / _desc.stepDurationBeats));
		_material->SetTextureUVOffset(stepIndex % 2 == 0 ? _desc.firstOffset : _desc.secondOffset);
	}
}
