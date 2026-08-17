#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	struct HitEvent;

	struct ReverbChangedEvent : EventType
	{
		float previousReverb = 0.f;
		float currentReverb = 0.f;
		float maxReverb = 0.f;
	};

	class ReverbComponent final : public Component
	{
	public:
		explicit ReverbComponent(float maxReverb);

		float	AddReverb(float amount);
		bool	ConsumeReverb(float amount);
		void	SetReverb(float reverb);
		void	HandleAttackHit(const HitEvent& event);
		void	SetReverbGainPerHit(float reverbGainPerHit);

		float	GetReverb() const { return _reverb; }
		float	GetMaxReverb() const { return _maxReverb; }
		float	GetReverbRatio() const { return _reverb / _maxReverb; }
		float	GetReverbGainPerHit() const { return _reverbGainPerHit; }
		bool	IsFull() const { return _reverb >= _maxReverb; }

		EventPublisher<ReverbComponent, ReverbChangedEvent> OnReverbChanged;

	private:
		void PublishReverbChanged(float previousReverb);

		float _maxReverb = 1.f;
		float _reverb = 0.f;
		float _reverbGainPerHit = 5.f;
	};
}
