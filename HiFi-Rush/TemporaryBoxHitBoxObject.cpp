#include "TemporaryBoxHitBoxObject.h"

#include "BeatSystem.h"
#include "BoxCollider3DComponent.h"
#include "HiFiRushStatics.h"
#include "HitBoxComponent.h"
#include "TransformComponent.h"

namespace gm
{
	TemporaryHitBoxObject::TemporaryHitBoxObject(const TemporaryBoxHitBoxDesc& desc)
		: _onHit(desc.onHit), _lifetime(desc.lifetime), _activationDelayBeats(desc.activationDelayBeats)
	{
		GM_ASSERT_RETURN(desc.lifetime > 0.f, "Temporary Box HitBox Lifetime은 0보다 커야 합니다.");
		GM_ASSERT_RETURN(desc.activationDelayBeats >= 0.f, "Temporary Box HitBox 활성화 지연은 0 Beat 이상이어야 합니다.");

		GetTransform()->SetWorldMatrix(desc.world);

		BoxCollider3DComponent* collider = AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN(collider, "TemporaryBoxHitBoxObject의 BoxCollider3DComponent 생성에 실패했습니다.");
		if (desc.colliderId.empty() == false)
			collider->SetColliderId(desc.colliderId);
		collider->SetLocalCenter(desc.localCenter);
		collider->SetSize(desc.size);
		collider->SetCollisionLayer(desc.collisionLayer);
		collider->SetCollisionMask(desc.collisionMask);

		_hitBox = AddComponent<HitBoxComponent>(*collider);
		GM_ASSERT_RETURN(_hitBox, "TemporaryBoxHitBoxObject의 HitBoxComponent 생성에 실패했습니다.");
		_hitBox->SetDamageInfo(desc.damageInfo);
		_hitBox->SetRehitInterval(desc.rehitInterval);
	}

	void TemporaryHitBoxObject::OnInitialize()
	{
		GM_ASSERT_RETURN(_hitBox, "TemporaryBoxHitBoxObject에 HitBoxComponent가 필요합니다.");
		if (_onHit)
			_hitBox->OnHit.Subscribe(_hitConnection, _onHit);

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (_activationDelayBeats > 0.f && beatSystem.HasPlaybackTime())
		{
			_activationBeat = beatSystem.GetCurrentBeat() + _activationDelayBeats;
			return;
		}

		_hitBox->BeginAttack();
		_hasActivated = true;
	}

	void TemporaryHitBoxObject::OnTick(float deltaTime)
	{
		if (_hitBox == nullptr)
		{
			Destroy();
			return;
		}

		if (_hasActivated == false)
		{
			const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
			if (_activationBeat.has_value() && beatSystem.HasPlaybackTime() && beatSystem.GetCurrentBeat() < _activationBeat.value())
				return;

			_hitBox->BeginAttack();
			_hasActivated = true;
		}

		_elapsedTime += deltaTime;
		if (_elapsedTime < _lifetime)
			return;

		_hitBox->EndAttack();
		Destroy();
	}
}
