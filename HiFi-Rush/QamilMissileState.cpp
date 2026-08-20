#include "QamilMissileState.h"

#include "Application.h"
#include "GameObject.h"
#include "QamilMissileObject.h"
#include "QamilResources.h"
#include "QamilStateMachineComponent.h"
#include "Random.h"
#include "Resources.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SocketComponent.h"
#include "Texture.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		constexpr float QamilMissileReadyPlayRateScale = 55.f / 60.f;
		constexpr float QamilMissileGoPlayRateScale = 170.f / 165.f;
	}

	void QamilMissileState::Enter(QamilStateContext& context)
	{
		_missiles.clear();
		GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::MissileReady, QamilMissileReadyPlayRateScale), "Qamil Missile Ready Animation 재생에 실패했습니다.");
	}

	void QamilMissileState::Tick(QamilStateContext& context, float)
	{
		if (IsAnimationCompleted(context) == false)
			return;

		switch (_currentAnimationId)
		{
		case QamilAnimationId::MissileReady:
			GM_ASSERT_RETURN(SpawnMissile(context, 0), "첫 번째 Qamil Missile 생성에 실패했습니다.");
			GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::Missile1), "Qamil Missile1 Animation 재생에 실패했습니다.");
			break;
		case QamilAnimationId::Missile1:
			GM_ASSERT_RETURN(SpawnMissile(context, 1), "두 번째 Qamil Missile 생성에 실패했습니다.");
			GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::Missile2), "Qamil Missile2 Animation 재생에 실패했습니다.");
			break;
		case QamilAnimationId::Missile2:
			GM_ASSERT_RETURN(SpawnMissile(context, 2), "세 번째 Qamil Missile 생성에 실패했습니다.");
			GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::Missile3), "Qamil Missile3 Animation 재생에 실패했습니다.");
			break;
		case QamilAnimationId::Missile3:
			GM_ASSERT_RETURN(SpawnMissile(context, 3), "네 번째 Qamil Missile 생성에 실패했습니다.");
			GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::Missile4), "Qamil Missile4 Animation 재생에 실패했습니다.");
			break;
		case QamilAnimationId::Missile4:
			GM_ASSERT_RETURN(PlaySequenceAnimation(context, QamilAnimationId::MissileGo, QamilMissileGoPlayRateScale), "Qamil Missile Go Animation 재생에 실패했습니다.");
			break;
		case QamilAnimationId::MissileGo:
			context.stateMachine->ChangeState(QamilStateId::Idle);
			break;
		default:
			GM_ASSERT(false, "Qamil Missile State의 Animation 순서가 유효하지 않습니다.");
			break;
		}
	}

	void QamilMissileState::Exit(QamilStateContext& context)
	{
		for (const WeakGameObjectPtr& missile : _missiles)
		{
			GameObject* missileObject = missile.Get();
			if (missileObject)
				missileObject->Destroy();
		}
		_missiles.clear();
		_currentAnimationId = QamilAnimationId::MissileReady;
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
	}

	bool QamilMissileState::PlaySequenceAnimation(QamilStateContext& context, QamilAnimationId animationId, float playRateScale)
	{
		if (PlayBeatSyncedAnimation(context, animationId, false, 0.f) == false)
			return false;
		_currentAnimationId = animationId;
		context.animatorComponent->SetPlayRate(GetBasePlayRate(context) * playRateScale);
		return true;
	}

	bool QamilMissileState::SpawnMissile(QamilStateContext& context, uint32 missileIndex)
	{
		if (missileIndex >= QamilMissileSocketBindings.size() || context.stateMachine == nullptr || context.transformComponent == nullptr)
			return false;
		Scene* scene = context.stateMachine->GetOwner().GetScene();
		if (scene == nullptr)
			return false;

		const QamilMissileSocketBinding& socketBinding = QamilMissileSocketBindings[missileIndex];
		Vector3 launchPosition{};
		Vector3 directionSourcePosition{};
		if (GetSocketPosition(context, socketBinding.launchSocketName, launchPosition) == false || GetSocketPosition(context, socketBinding.directionSourceSocketName, directionSourcePosition) == false)
			return false;

		Resources& resources = APPLICATION.GetResources();
		QamilMissileDesc desc{};
		desc.startPosition = launchPosition;
		desc.initialDirection = launchPosition - directionSourcePosition;
		desc.arenaCenter = context.transformComponent->GetPosition();
		desc.targetDirection = desc.arenaCenter - GetCurrentPlatformPosition(context) + Vector3{ 0.f, -1.f, 0.f };
		desc.warningPosition = CreateWarningPosition(desc.arenaCenter, missileIndex);
		desc.target = context.target;
		desc.skeletalMesh = resources.Find<SkeletalMesh>(QamilMissileSkeletalMeshResourceKey);
		desc.animation = resources.Find<SkeletalAnimationClip>(QamilMissileAnimationResourceKey);
		desc.warningTexture = resources.Find<Texture>(QamilMissileWarningTextureResourceKey);
		if (desc.skeletalMesh == nullptr || desc.animation == nullptr || desc.warningTexture == nullptr)
			return false;
		QamilMissileObject* missile = scene->SpawnGameObject<QamilMissileObject>(desc);
		if (missile == nullptr)
			return false;
		_missiles.push_back(missile->GetWeakPtr());
		return true;
	}

	Vector3 QamilMissileState::CreateWarningPosition(const Vector3& arenaCenter, uint32 missileIndex) const
	{
		Vector3 position = arenaCenter;
		position.y += 0.01f * static_cast<float>(missileIndex + 1);
		switch (missileIndex)
		{
		case 0:
			position.x += Math::RandomFloat(-5.f, 5.f);
			position.z += Math::RandomFloat(2.f, 8.f);
			break;
		case 1:
			position.x += Math::RandomFloat(-8.f, -2.f);
			position.z += Math::RandomFloat(-5.f, 5.f);
			break;
		case 2:
			position.x += Math::RandomFloat(-5.f, 5.f);
			position.z += Math::RandomFloat(-8.f, -2.f);
			break;
		case 3:
			position.x += Math::RandomFloat(2.f, 8.f);
			position.z += Math::RandomFloat(-5.f, 5.f);
			break;
		}
		return position;
	}

	bool QamilMissileState::GetSocketPosition(const QamilStateContext& context, const wchar_t* socketName, Vector3& outPosition) const
	{
		if (context.socketComponent == nullptr || context.socketComponent->HasSocket(socketName) == false)
			return false;
		Matrix socketWorld = context.socketComponent->GetSocketAnchorWorldMatrix(socketName);
		Vector3 scale{};
		Quaternion rotation{};
		return socketWorld.Decompose(scale, rotation, outPosition);
	}
}
