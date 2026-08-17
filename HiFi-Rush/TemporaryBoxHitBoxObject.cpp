#include "TemporaryBoxHitBoxObject.h"

#include "BoxCollider3DComponent.h"
#include "HitBoxComponent.h"
#include "TransformComponent.h"

namespace gm
{
	TemporaryHitBoxObject::TemporaryHitBoxObject(const TemporaryBoxHitBoxDesc& desc)
		: _onHit(desc.onHit), _lifetime(desc.lifetime)
	{
		GM_ASSERT_RETURN(desc.lifetime > 0.f, "Temporary Box HitBox Lifetime은 0보다 커야 합니다.");

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
		_hitBox->BeginAttack();
	}

	void TemporaryHitBoxObject::OnTick(float deltaTime)
	{
		if (_hitBox == nullptr)
		{
			Destroy();
			return;
		}

		_elapsedTime += deltaTime;
		if (_elapsedTime < _lifetime)
			return;

		_hitBox->EndAttack();
		Destroy();
	}
}
