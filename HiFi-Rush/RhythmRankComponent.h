#pragma once

#include "Component.h"
#include "Event.h"
#include "RhythmRankTypes.h"

namespace gm
{
	struct RhythmJudgeResult;

	struct RhythmRankChangedEvent : EventType
	{
		RhythmRank previousRank = RhythmRank::C;
		RhythmRank currentRank = RhythmRank::C;
		float previousGauge = 0.f;
		float currentGauge = 0.f;
		float gaugeRatio = 0.f;
	};

	class RhythmRankComponent final : public Component
	{
	public:
		RhythmRankComponent() = default;

		void	AddGauge(float amount);
		void	SetDecayPerSecond(float decayPerSecond);
		void	SetAttackGaugeGain(float gaugeGain);
		void	SetMovementGaugeGain(float gaugeGain);

		RhythmRank	GetRank() const { return _rank; }
		float		GetGauge() const { return _gauge; }
		float		GetGaugeRatio() const { return _gauge / GaugePerRank; }
		float		GetDecayPerSecond() const { return _decayPerSecond; }
		float		GetAttackGaugeGain() const { return _attackGaugeGain; }
		float		GetMovementGaugeGain() const { return _movementGaugeGain; }

		EventPublisher<RhythmRankComponent, RhythmRankChangedEvent> OnRhythmRankChanged;

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void HandleRhythmActionStarted(const RhythmJudgeResult& result);
		void RemoveGauge(float amount);
		void PublishRhythmRankChanged(RhythmRank previousRank, float previousGauge);

		static constexpr float GaugePerRank = 100.f;

		RhythmRank		_rank = RhythmRank::C;
		float			_gauge = 10.f;
		float			_decayPerSecond = 1.75f;
		float			_attackGaugeGain = 7.f;
		float			_movementGaugeGain = 1.5f;
		EventConnection	_rhythmActionStartedConnection{};
	};
}
