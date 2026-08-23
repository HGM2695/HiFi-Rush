#include "BeatMaterialBrightnessPulseComponent.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "Material.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"

#include <cmath>

namespace gm
{
	BeatMaterialBrightnessPulseComponent::BeatMaterialBrightnessPulseComponent(const BeatSystem& beatSystem, const BeatMaterialBrightnessPulseDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatMaterialBrightnessPulseComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.patternLengthBeats > 0 && _desc.patternLengthBeats <= 32, "BeatMaterialBrightnessPulseComponent의 Pattern Length는 1부터 32 사이여야 합니다.");
		GM_ASSERT_RETURN(_desc.activeBeatMask != 0, "BeatMaterialBrightnessPulseComponent의 Active Beat Mask가 비어 있습니다.");
		GM_ASSERT_RETURN(_desc.minimumBrightness >= 0.f && _desc.maximumBrightness >= _desc.minimumBrightness, "BeatMaterialBrightnessPulseComponent의 Brightness 범위가 유효하지 않습니다.");
		GM_ASSERT_RETURN(ResolveMaterial(), "BeatMaterialBrightnessPulseComponent의 Material을 찾을 수 없습니다. slot=%u", _desc.materialSlot);
		_initialColorMultiplier = _material->GetColorData().colorMultiplier;
		UpdateBrightness();
	}

	void BeatMaterialBrightnessPulseComponent::OnTick(float)
	{
		UpdateBrightness();
	}

	bool BeatMaterialBrightnessPulseComponent::ResolveMaterial()
	{
		StaticMeshComponent* staticMesh = GetOwner().GetComponent<StaticMeshComponent>();
		SkeletalMeshComponent* skeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL((staticMesh != nullptr) != (skeletalMesh != nullptr), false, "BeatMaterialBrightnessPulseComponent는 하나의 Static 또는 Skeletal Mesh Component가 필요합니다.");
		_material = staticMesh ? staticMesh->GetMaterial(_desc.materialSlot) : skeletalMesh->GetMaterial(_desc.materialSlot);
		return _material != nullptr;
	}

	void BeatMaterialBrightnessPulseComponent::UpdateBrightness()
	{
		if (_material == nullptr)
			return;

		float brightness = _desc.minimumBrightness;
		if (_beatSystem.HasPlaybackTime())
		{
			const float currentBeat = _beatSystem.GetCurrentBeat();
			const int64 beatIndex = static_cast<int64>(std::floor(currentBeat));
			const int64 patternLength = static_cast<int64>(_desc.patternLengthBeats);
			const uint32 patternBeatIndex = static_cast<uint32>((beatIndex % patternLength + patternLength) % patternLength);
			if ((_desc.activeBeatMask & (uint32{ 1 } << patternBeatIndex)) != 0)
			{
				const float pulse = BeatMath::EvaluateBeatIntervalPulse(currentBeat, 1.f);
				brightness += (_desc.maximumBrightness - _desc.minimumBrightness) * pulse;
			}
		}

		_material->SetColorMultiplier(Color{
			_initialColorMultiplier.x * brightness,
			_initialColorMultiplier.y * brightness,
			_initialColorMultiplier.z * brightness,
			_initialColorMultiplier.w });
	}
}
