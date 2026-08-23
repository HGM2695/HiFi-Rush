#include "BeatTextureUVScrollComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "Material.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"

#include <cmath>

namespace gm
{
	BeatTextureUVScrollComponent::BeatTextureUVScrollComponent(const BeatSystem& beatSystem, const BeatTextureUVScrollDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatTextureUVScrollComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(ResolveMaterial(), "BeatTextureUVScrollComponent의 Material을 찾을 수 없습니다. slot=%u", _desc.materialSlot);
		UpdateTextureUVOffset();
	}

	void BeatTextureUVScrollComponent::OnTick(float)
	{
		UpdateTextureUVOffset();
	}

	bool BeatTextureUVScrollComponent::ResolveMaterial()
	{
		StaticMeshComponent* staticMesh = GetOwner().GetComponent<StaticMeshComponent>();
		SkeletalMeshComponent* skeletalMesh = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL((staticMesh != nullptr) != (skeletalMesh != nullptr), false, "BeatTextureUVScrollComponent는 하나의 Static 또는 Skeletal Mesh Component가 필요합니다.");
		_material = staticMesh ? staticMesh->GetMaterial(_desc.materialSlot) : skeletalMesh->GetMaterial(_desc.materialSlot);
		return _material != nullptr;
	}

	void BeatTextureUVScrollComponent::UpdateTextureUVOffset()
	{
		if (_material == nullptr)
			return;

		const float currentBeat = _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() : 0.f;
		Vector2 offset = _desc.offsetPerBeat * currentBeat;
		offset.x -= std::floor(offset.x);
		offset.y -= std::floor(offset.y);
		_material->SetTextureUVOffset(offset);
	}
}
