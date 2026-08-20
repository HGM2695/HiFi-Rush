#include "QamilMoveState.h"

#include "MathUtil.h"
#include "QamilStateMachineComponent.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float QamilPlatformRotation = Math::GM_PI * 0.5f;
	}

	void QamilMoveState::Enter(QamilStateContext& context)
	{
		_phase = Phase::Start;
		_isClockwise = SelectClockwiseDirection(context);
		PlayBeatSyncedAnimation(context, _isClockwise ? QamilAnimationId::MoveLeftStart : QamilAnimationId::MoveRightStart, false);
	}

	void QamilMoveState::Tick(QamilStateContext& context, float)
	{
		if (_phase == Phase::Move)
		{
			const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
			const float ratio = clip && clip->GetLength() > 0.f ? std::clamp(context.animatorComponent->GetPlayTime() / clip->GetLength(), 0.f, 1.f) : 1.f;
			const float smoothRatio = ratio * ratio * (3.f - 2.f * ratio);
			context.transformComponent->SetRotationY(_startRotationY + (_targetRotationY - _startRotationY) * smoothRatio);
		}

		if (IsAnimationCompleted(context) == false)
			return;

		switch (_phase)
		{
		case Phase::Start:
			BeginMove(context);
			break;
		case Phase::Move:
			context.transformComponent->SetRotationY(_targetRotationY);
			BeginEnd(context);
			break;
		case Phase::End:
			context.stateMachine->ChangeState(context.stateMachine->SelectNextWideAttackState());
			break;
		}
	}

	void QamilMoveState::BeginMove(QamilStateContext& context)
	{
		_phase = Phase::Move;
		const Vector3 lookDirection = Math::GetLookVector(context.transformComponent->GetRotation());
		_startRotationY = std::atan2(lookDirection.x, lookDirection.z);
		_targetRotationY = _startRotationY + (_isClockwise ? QamilPlatformRotation : -QamilPlatformRotation);
		PlayBeatSyncedAnimation(context, _isClockwise ? QamilAnimationId::MoveLeft : QamilAnimationId::MoveRight, false);
	}

	void QamilMoveState::BeginEnd(QamilStateContext& context)
	{
		_phase = Phase::End;
		PlayBeatSyncedAnimation(context, _isClockwise ? QamilAnimationId::MoveLeftEnd : QamilAnimationId::MoveRightEnd, false);
	}

}
