#include "ChiAudioComponent.h"

#include "AudioStatics.h"
#include "ChiStateMachineComponent.h"
#include "CombatTypes.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushAudio.h"
#include "RhythmInputJudge.h"

namespace gm
{
	void ChiAudioComponent::OnInitialize()
	{
		ChiStateMachineComponent* stateMachine = GetOwner().GetComponent<ChiStateMachineComponent>();
		HealthComponent* health = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(stateMachine && health, "ChiAudioComponent에는 ChiStateMachineComponent와 HealthComponent가 필요합니다.");

		stateMachine->OnRhythmActionStarted.Subscribe(_rhythmActionConnection, [this](const RhythmJudgeResult& result) { HandleRhythmActionStarted(result); });
		health->OnDamaged.Subscribe(_damagedConnection, [this](const HitEvent& event) { HandleDamaged(event); });
	}

	void ChiAudioComponent::HandleRhythmActionStarted(const RhythmJudgeResult& result)
	{
		if (result.judgeGrade == RhythmJudgeGrade::OffBeat)
			return;

		switch (result.type)
		{
		case RhythmInputType::WeakAttack:
		case RhythmInputType::StrongAttack:
			PlaySound2D(HiFiRushSound::ChiAttackVoices[_attackVoiceIndex]);
			_attackVoiceIndex = (_attackVoiceIndex + 1) % HiFiRushSound::ChiAttackVoices.size();
			break;
		case RhythmInputType::Jump:
			PlaySound2D(HiFiRushSound::ChiJumpVoices[_jumpVoiceIndex]);
			_jumpVoiceIndex = (_jumpVoiceIndex + 1) % HiFiRushSound::ChiJumpVoices.size();
			break;
		case RhythmInputType::Dash:
			PlaySound2D(HiFiRushSound::ChiDash);
			break;
		default:
			break;
		}
	}

	void ChiAudioComponent::HandleDamaged(const HitEvent& event)
	{
		if (event.damageResult.state == DamageState::Applied && event.hitBox && event.hurtBox)
			PlaySound2D(HiFiRushSound::ChiDamage, 0.5f);
	}
}
