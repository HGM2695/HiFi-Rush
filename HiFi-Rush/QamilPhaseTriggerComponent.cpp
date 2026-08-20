#include "QamilPhaseTriggerComponent.h"

#include "GameObject.h"
#include "GameplayScene.h"
#include "QamilStateMachineComponent.h"
#include "QamilStateTypes.h"
#include "TriggerSystem.h"

namespace gm
{
	void QamilPhaseTriggerComponent::OnInitialize()
	{
		_stateMachine = GetOwner().GetComponent<QamilStateMachineComponent>();
		GM_ASSERT_RETURN(_stateMachine, "QamilPhaseTriggerComponent는 QamilStateMachineComponent가 필요합니다.");

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "QamilPhaseTriggerComponent는 GameplayScene에서만 사용할 수 있습니다.");
		_triggerSystem = &scene->GetTriggerSystem();
		_stateMachine->OnPhaseChanged.Subscribe(_phaseChangedConnection, [this](const QamilPhaseChangedEvent& event) { HandlePhaseChanged(event); });
	}

	void QamilPhaseTriggerComponent::HandlePhaseChanged(const QamilPhaseChangedEvent& event)
	{
		GM_ASSERT_RETURN(_triggerSystem, "Qamil Phase Trigger를 실행할 TriggerSystem이 유효하지 않습니다.");

		if (event.previousPhase < QamilPhase::Phase2 && event.currentPhase >= QamilPhase::Phase2)
			GM_ASSERT_RETURN(_triggerSystem->Activate(QamilPhase2TriggerId), "Qamil Phase 2 Trigger 활성화에 실패했습니다.");
		if (event.previousPhase < QamilPhase::Phase3 && event.currentPhase >= QamilPhase::Phase3)
			GM_ASSERT_RETURN(_triggerSystem->Activate(QamilPhase3TriggerId), "Qamil Phase 3 Trigger 활성화에 실패했습니다.");
	}
}
