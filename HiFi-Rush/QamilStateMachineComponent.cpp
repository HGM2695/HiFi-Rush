#include "QamilStateMachineComponent.h"

#include "CombatTypes.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HealthComponent.h"
#include "HiFiRushStatics.h"
#include "QamilLifecycleState.h"
#include "QamilMissileState.h"
#include "QamilMoveState.h"
#include "QamilNormalAttackState.h"
#include "QamilSpecialAttackState.h"
#include "QamilState.h"
#include "QamilWideAttackState.h"
#include "Random.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMeshComponent.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

#include <array>

namespace gm
{
	namespace
	{
		constexpr float QamilPhase2StartHealthRatio = 0.75f;
		constexpr float QamilPhase3StartHealthRatio = 0.5f;
	}

	QamilStateMachineComponent::QamilStateMachineComponent() = default;
	QamilStateMachineComponent::~QamilStateMachineComponent() = default;

	bool QamilStateMachineComponent::StartCombat()
	{
		if (_context.healthComponent->IsDead())
			return false;
		GM_ASSERT_RETURN_VAL(ResolveTarget(), false, "Qamil 전투 대상을 찾을 수 없습니다.");

		_isCombatActive = true;
		_wideAttackIndex = 0;
		_previousAttackStateId = QamilStateId::None;
		_context.healthComponent->SetInvincible(false);
		_context.animatorComponent->Resume();
		return ChangeState(QamilStateId::Idle);
	}

	bool QamilStateMachineComponent::StopCombat()
	{
		_isCombatActive = false;
		_context.healthComponent->SetInvincible(true);
		return ChangeState(QamilStateId::Inactive);
	}

	bool QamilStateMachineComponent::ChangeState(QamilStateId nextStateId, bool restart)
	{
		if (_currentStateId == nextStateId && restart == false)
			return true;

		QamilState* nextState = FindState(nextStateId);
		GM_ASSERT_RETURN_VAL(nextState, false, "등록되지 않은 Qamil State로 전환할 수 없습니다.");

		QamilState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Exit(_context);

		_currentStateId = nextStateId;
		nextState->Enter(_context);
		return true;
	}

	QamilStateId QamilStateMachineComponent::SelectNextAttackState()
	{
		const QamilStateId closeAttackStateId = _currentPhase == QamilPhase::Phase3 ? QamilStateId::Chain : QamilStateId::NormalAttack;
		if (_previousAttackStateId == closeAttackStateId || Math::RandomInt(0, 99) < 40)
			return SelectNextWideAttackState();

		_previousAttackStateId = closeAttackStateId;
		return closeAttackStateId;
	}

	QamilStateId QamilStateMachineComponent::SelectNextWideAttackState()
	{
		constexpr std::array Phase1WideAttacks{ QamilStateId::Stump, QamilStateId::Sweep };
		constexpr std::array Phase2WideAttacks{ QamilStateId::Stump, QamilStateId::Sweep, QamilStateId::Missile };
		constexpr std::array Phase3WideAttacks{ QamilStateId::Stump, QamilStateId::Sweep, QamilStateId::Missile, QamilStateId::Laser };
		QamilStateId stateId = QamilStateId::None;
		if (_currentPhase == QamilPhase::Phase1)
			stateId = Phase1WideAttacks[_wideAttackIndex % Phase1WideAttacks.size()];
		else if (_currentPhase == QamilPhase::Phase2)
			stateId = Phase2WideAttacks[_wideAttackIndex % Phase2WideAttacks.size()];
		else
			stateId = Phase3WideAttacks[_wideAttackIndex % Phase3WideAttacks.size()];

		++_wideAttackIndex;
		_previousAttackStateId = stateId;
		return stateId;
	}

	void QamilStateMachineComponent::OnInitialize()
	{
		_context.beatSystem = &HiFiRushStatics::GetBeatSystem();
		_context.stateMachine = this;
		_context.healthComponent = GetOwner().GetComponent<HealthComponent>();
		_context.animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		_context.skeletalMeshComponent = GetOwner().GetComponent<SkeletalMeshComponent>();
		_context.socketComponent = GetOwner().GetComponent<SocketComponent>();
		_context.transformComponent = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_context.healthComponent, "QamilStateMachineComponent는 HealthComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.animatorComponent, "QamilStateMachineComponent는 SkeletalAnimatorComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.skeletalMeshComponent, "QamilStateMachineComponent는 SkeletalMeshComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.socketComponent, "QamilStateMachineComponent는 SocketComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.transformComponent, "QamilStateMachineComponent는 TransformComponent가 필요합니다.");
		GM_ASSERT_RETURN(RegisterStates(), "Qamil State 등록에 실패했습니다.");
		ResolveTarget();

		_currentPhase = CalculatePhase(_context.healthComponent->GetHealth(), _context.healthComponent->GetMaxHealth());
		_context.healthComponent->SetInvincible(true);
		_context.healthComponent->OnHealthChanged.Subscribe(_healthChangedConnection, [this](const HealthChangedEvent& event) { HandleHealthChanged(event); });
		_context.healthComponent->OnDeath.Subscribe(_deathConnection, [this](const HitEvent& event) { HandleDeath(event); });
		ChangeState(QamilStateId::Inactive);
	}

	void QamilStateMachineComponent::OnTick(float deltaTime)
	{
		QamilState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Tick(_context, deltaTime);
	}

	bool QamilStateMachineComponent::RegisterState(std::unique_ptr<QamilState> state)
	{
		GM_ASSERT_RETURN_VAL(state, false, "등록할 Qamil State가 유효하지 않습니다.");
		const QamilStateId stateId = state->GetStateId();
		GM_ASSERT_RETURN_VAL(stateId != QamilStateId::None && stateId != QamilStateId::Count, false, "등록할 수 없는 Qamil State ID입니다.");
		GM_ASSERT_RETURN_VAL(_states.contains(stateId) == false, false, "Qamil State ID가 중복되었습니다.");
		_states.emplace(stateId, std::move(state));
		return true;
	}

	bool QamilStateMachineComponent::RegisterStates()
	{
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilInactiveState>()), false, "Qamil Inactive State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilIdleState>()), false, "Qamil Idle State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilMoveState>()), false, "Qamil Move State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilNormalAttackState>()), false, "Qamil Normal Attack State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilStumpState>()), false, "Qamil Stump State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilSweepState>()), false, "Qamil Sweep State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilMissileState>()), false, "Qamil Missile State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilLaserState>()), false, "Qamil Laser State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilChainState>()), false, "Qamil Chain State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<QamilDeadState>()), false, "Qamil Dead State 등록에 실패했습니다.");
		return true;
	}

	bool QamilStateMachineComponent::ResolveTarget()
	{
		GameplayScene* gameplayScene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		if (gameplayScene == nullptr || gameplayScene->GetPlayer().IsValid() == false)
			return false;

		_context.triggerSystem = &gameplayScene->GetTriggerSystem();
		_context.target = gameplayScene->GetPlayer();
		return true;
	}

	QamilState* QamilStateMachineComponent::FindState(QamilStateId stateId) const
	{
		const auto iter = _states.find(stateId);
		return iter != _states.end() ? iter->second.get() : nullptr;
	}

	QamilPhase QamilStateMachineComponent::CalculatePhase(int32 health, int32 maxHealth) const
	{
		const float healthRatio = static_cast<float>(health) / static_cast<float>(maxHealth);

		if (healthRatio <= QamilPhase3StartHealthRatio)
			return QamilPhase::Phase3;
		if (healthRatio <= QamilPhase2StartHealthRatio)
			return QamilPhase::Phase2;

		return QamilPhase::Phase1;
	}

	void QamilStateMachineComponent::HandleHealthChanged(const HealthChangedEvent& event)
	{
		const QamilPhase nextPhase = CalculatePhase(event.currentHealth, event.maxHealth);
		if (_currentPhase == nextPhase)
			return;

		QamilPhaseChangedEvent phaseChangedEvent{};
		phaseChangedEvent.previousPhase = _currentPhase;
		phaseChangedEvent.currentPhase = nextPhase;
		_currentPhase = nextPhase;
		OnPhaseChanged.Publish(phaseChangedEvent);
	}

	void QamilStateMachineComponent::HandleDeath(const HitEvent&)
	{
		_isCombatActive = false;
		ChangeState(QamilStateId::Dead);
		QamilDefeatedEvent defeatedEvent{};
		OnDefeated.Publish(defeatedEvent);
	}
}
