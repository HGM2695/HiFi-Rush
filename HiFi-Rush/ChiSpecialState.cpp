#include "ChiSpecialState.h"
#include "ChiStateMachineComponent.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "Scene.h"
#include "TemporaryBoxHitBoxObject.h"
#include "TransformComponent.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	ChiHibikiReadyState::ChiHibikiReadyState()
		: ChiState(ChiStateId::HibikiReady, ChiAnimationClipId::HibikiReady)
	{
	}

	void ChiHibikiReadyState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::HibikiAttack);
	}

	ChiHibikiAttackState::ChiHibikiAttackState()
		: ChiAttackState(ChiStateId::HibikiAttack, ChiAnimationClipId::HibikiAttack)
	{
	}

	void ChiHibikiAttackState::Enter(ChiStateContext& context)
	{
		ChiAttackState::Enter(context);
		_hasSpawnedAreaHitBox = false;
	}

	void ChiHibikiAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		RestoreBasePlayRateAfterImpact(context);
		constexpr float AnimationTicksPerBeat = 15.f;
		constexpr float AreaHitBoxStartBeat = 1.f + 2.f / AnimationTicksPerBeat;
		if (_hasSpawnedAreaHitBox == false && GetAnimationBeat(context) >= AreaHitBoxStartBeat)
		{
			_hasSpawnedAreaHitBox = true;
			GM_ASSERT(SpawnAreaHitBox(context), "히비키 광역 HitBox GameObject 생성에 실패했습니다.");
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	bool ChiHibikiAttackState::SpawnAreaHitBox(ChiStateContext& context) const
	{
		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		TransformComponent* transform = owner.GetTransform();
		GM_ASSERT_RETURN_VAL(scene && transform, false, "히비키 광역 HitBox 생성에 필요한 Scene 또는 Transform이 없습니다.");

		TemporaryBoxHitBoxDesc desc{};
		desc.world = transform->GetWorldMatrix();
		desc.colliderId = L"HibikiAttack";
		desc.localCenter = Vector3{ 0.f, 2.5f, 0.f };
		desc.size = Vector3{ 40.f, 6.f, 40.f };
		desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
		desc.collisionMask = HiFiRushCollisionMask::PlayerAttackTargets;
		desc.damageInfo.amount = 10;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.lifetime = 1.f;
		return scene->SpawnGameObject<TemporaryHitBoxObject>(desc) != nullptr;
	}
}
