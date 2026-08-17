#include "RhythmRankComponent.h"

#include "ChiStateMachineComponent.h"
#include "GameObject.h"
#include "RhythmInputJudge.h"

#include <algorithm>

namespace gm
{
	void RhythmRankComponent::AddGauge(float amount)
	{
		if (amount == 0.f || (_rank == RhythmRank::S && _gauge >= GaugePerRank))
			return;

		const RhythmRank previousRank = _rank;
		const float previousGauge = _gauge;
		_gauge += amount;

		while (_gauge >= GaugePerRank && _rank != RhythmRank::S)
		{
			_gauge -= GaugePerRank;
			_rank = static_cast<RhythmRank>(GetRhythmRankIndex(_rank) + 1);
		}

		PublishRhythmRankChanged(previousRank, previousGauge);
	}

	void RhythmRankComponent::SetDecayPerSecond(float decayPerSecond)
	{
		GM_ASSERT_RETURN(decayPerSecond >= 0.f, "Rhythm Rank Gauge 감소량은 0 이상이어야 합니다.");
		_decayPerSecond = decayPerSecond;
	}

	void RhythmRankComponent::SetAttackGaugeGain(float gaugeGain)
	{
		GM_ASSERT_RETURN(gaugeGain >= 0.f, "공격의 Rhythm Rank Gauge 획득량은 0 이상이어야 합니다.");
		_attackGaugeGain = gaugeGain;
	}

	void RhythmRankComponent::SetMovementGaugeGain(float gaugeGain)
	{
		GM_ASSERT_RETURN(gaugeGain >= 0.f, "이동 행동의 Rhythm Rank Gauge 획득량은 0 이상이어야 합니다.");
		_movementGaugeGain = gaugeGain;
	}

	void RhythmRankComponent::OnInitialize()
	{
		ChiStateMachineComponent* stateMachine = GetOwner().GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(stateMachine, "RhythmRankComponent에는 ChiStateMachineComponent가 필요합니다.");

		stateMachine->OnRhythmActionStarted.Subscribe(_rhythmActionStartedConnection,
			[this](const RhythmJudgeResult& result)
			{
				HandleRhythmActionStarted(result);
			});
	}

	void RhythmRankComponent::OnTick(float deltaTime)
	{
		if (deltaTime > 0.f && _decayPerSecond > 0.f)
			RemoveGauge(_decayPerSecond * deltaTime);
	}

	void RhythmRankComponent::HandleRhythmActionStarted(const RhythmJudgeResult& result)
	{
		if (result.judgeGrade == RhythmJudgeGrade::OffBeat)
			return;

		switch (result.type)
		{
		case RhythmInputType::WeakAttack:
		case RhythmInputType::StrongAttack:
			AddGauge(_attackGaugeGain);
			break;

		case RhythmInputType::Jump:
		case RhythmInputType::Dash:
			AddGauge(_movementGaugeGain);
			break;

		default:
			break;
		}
	}

	void RhythmRankComponent::RemoveGauge(float amount)
	{
		if (amount <= 0.f || (_rank == RhythmRank::C && _gauge <= 0.f))
			return;

		const RhythmRank previousRank = _rank;
		const float previousGauge = _gauge;
		_gauge -= amount;

		while (_gauge < 0.f && _rank != RhythmRank::C)
		{
			_rank = static_cast<RhythmRank>(GetRhythmRankIndex(_rank) - 1);
			_gauge += GaugePerRank;
		}

		_gauge = (std::max)(_gauge, 0.f);
		PublishRhythmRankChanged(previousRank, previousGauge);
	}

	void RhythmRankComponent::PublishRhythmRankChanged(RhythmRank previousRank, float previousGauge)
	{
		RhythmRankChangedEvent event{};
		event.previousRank = previousRank;
		event.currentRank = _rank;
		event.previousGauge = previousGauge;
		event.currentGauge = _gauge;
		event.gaugeRatio = GetGaugeRatio();
		OnRhythmRankChanged.Publish(event);
	}
}
