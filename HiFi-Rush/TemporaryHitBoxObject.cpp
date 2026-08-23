#include "TemporaryHitBoxObject.h"

#include "BeatSystem.h"
#include "BoxCollider3DComponent.h"
#include "HiFiRushStatics.h"
#include "HitBoxComponent.h"
#include "SphereCollider3DComponent.h"
#include "TransformComponent.h"

namespace gm
{
	TemporaryHitBoxObject::TemporaryHitBoxObject(const TemporaryBoxHitBoxDesc& desc)
	{
		GetTransform()->SetWorldMatrix(desc.world);
		BoxCollider3DComponent* collider = AddComponent<BoxCollider3DComponent>();
		GM_ASSERT_RETURN(collider, "TemporaryHitBoxObject의 BoxCollider3DComponent 생성에 실패했습니다.");
		collider->SetLocalCenter(desc.localCenter);
		collider->SetSize(desc.size);
		ConfigureHitBox(*collider, desc);
	}

	TemporaryHitBoxObject::TemporaryHitBoxObject(const TemporarySphereHitBoxDesc& desc)
	{
		GM_ASSERT_RETURN(desc.radius > 0.f, "Temporary Sphere HitBox Radius는 0보다 커야 합니다.");
		GetTransform()->SetWorldMatrix(desc.world);
		SphereCollider3DComponent* collider = AddComponent<SphereCollider3DComponent>();
		GM_ASSERT_RETURN(collider, "TemporaryHitBoxObject의 SphereCollider3DComponent 생성에 실패했습니다.");
		collider->SetLocalCenter(desc.localCenter);
		collider->SetRadius(desc.radius);
		ConfigureHitBox(*collider, desc);
	}

	void TemporaryHitBoxObject::OnInitialize()
	{
		GM_ASSERT_RETURN(_hitBox, "TemporaryHitBoxObject에 HitBoxComponent가 필요합니다.");
		if (_onHit)
			_hitBox->OnHit.Subscribe(_hitConnection, _onHit);

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (_activationDelayBeats > 0.f && beatSystem.HasPlaybackTime())
		{
			_activationBeat = beatSystem.GetCurrentBeat() + _activationDelayBeats;
			return;
		}

		Activate();
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

			Activate();
		}

		_elapsedTime += deltaTime;
		if (_elapsedTime < _lifetime)
			return;

		_hitBox->EndAttack();
		Destroy();
	}

	void TemporaryHitBoxObject::Activate()
	{
		_hitBox->BeginAttack();
		if (_onActivated)
			_onActivated();
		_hasActivated = true;
	}

	void TemporaryHitBoxObject::ConfigureHitBox(Collider3DComponent& collider, const TemporaryHitBoxDesc& desc)
	{
		GM_ASSERT_RETURN(desc.lifetime > 0.f, "Temporary HitBox Lifetime은 0보다 커야 합니다.");
		GM_ASSERT_RETURN(desc.activationDelayBeats >= 0.f, "Temporary HitBox 활성화 지연은 0 Beat 이상이어야 합니다.");
		if (desc.colliderId.empty() == false)
			collider.SetColliderId(desc.colliderId);
		collider.SetCollisionLayer(desc.collisionLayer);
		collider.SetCollisionMask(desc.collisionMask);

		_hitBox = AddComponent<HitBoxComponent>(collider);
		GM_ASSERT_RETURN(_hitBox, "TemporaryHitBoxObject의 HitBoxComponent 생성에 실패했습니다.");
		_hitBox->SetDamageInfo(desc.damageInfo);
		_hitBox->SetRehitInterval(desc.rehitInterval);
		_hitBox->SetHitCondition(desc.hitCondition);
		_onActivated = desc.onActivated;
		_onHit = desc.onHit;
		_lifetime = desc.lifetime;
		_activationDelayBeats = desc.activationDelayBeats;
	}
}
