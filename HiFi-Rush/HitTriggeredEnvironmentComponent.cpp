#include "HitTriggeredEnvironmentComponent.h"
#include "CombatTypes.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HurtBoxComponent.h"
#include "TriggerSequenceSystem.h"

namespace gm
{
	HitTriggeredEnvironmentComponent::HitTriggeredEnvironmentComponent(const std::wstring& sequenceId)
		: _sequenceId(sequenceId)
	{
		GM_ASSERT(sequenceId.empty() == false, "HitTriggeredEnvironmentComponent의 Sequence ID는 비어 있을 수 없습니다.");
	}

	void HitTriggeredEnvironmentComponent::OnInitialize()
	{
		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "HitTriggeredEnvironmentComponent는 GameplayScene에서만 사용할 수 있습니다.");
		_triggerSequenceSystem = &scene->GetTriggerSequenceSystem();
		GM_ASSERT_RETURN(_triggerSequenceSystem->HasSequence(_sequenceId), "HitTriggeredEnvironmentComponent가 참조하는 Trigger Sequence가 없습니다. sequenceId=%ls", _sequenceId.c_str());

		GetOwner().ForEachComponent(
			[this](Component& component)
			{
				auto* hurtBox = dynamic_cast<HurtBoxComponent*>(&component);
				if (hurtBox == nullptr)
					return;

				auto connection = std::make_unique<EventConnection>();
				hurtBox->OnHurt.Subscribe(*connection,
					[this](const HitEvent& event)
					{
						HandleHurt(event);
					});
				_hurtConnections.push_back(std::move(connection));
			});

		GM_ASSERT_RETURN(_hurtConnections.empty() == false, "HitTriggeredEnvironmentComponent를 가진 GameObject에는 하나 이상의 HurtBoxComponent가 필요합니다.");
	}

	void HitTriggeredEnvironmentComponent::HandleHurt(const HitEvent& event)
	{
		if (event.damageResult.state != DamageState::Applied || _triggerSequenceSystem == nullptr)
			return;

		_triggerSequenceSystem->Activate(_sequenceId);
	}
}
