#include "ReverbComponent.h"
#include "CombatTypes.h"

#include <algorithm>

namespace gm
{
	ReverbComponent::ReverbComponent(float maxReverb)
		: _maxReverb((std::max)(maxReverb, 1.f))
	{
		GM_ASSERT(maxReverb > 0.f, "Max Reverb는 0보다 커야 합니다.");
	}

	float ReverbComponent::AddReverb(float amount)
	{
		GM_ASSERT_RETURN_VAL(amount >= 0.f, 0.f, "Reverb 획득량은 0 이상이어야 합니다.");
		if (amount == 0.f || IsFull())
			return 0.f;

		const float previousReverb = _reverb;
		_reverb = (std::min)(_reverb + amount, _maxReverb);
		PublishReverbChanged(previousReverb);
		return _reverb - previousReverb;
	}

	bool ReverbComponent::ConsumeReverb(float amount)
	{
		GM_ASSERT_RETURN_VAL(amount >= 0.f, false, "Reverb 소비량은 0 이상이어야 합니다.");
		if (amount > _reverb)
			return false;
		if (amount == 0.f)
			return true;

		const float previousReverb = _reverb;
		_reverb -= amount;
		PublishReverbChanged(previousReverb);
		return true;
	}

	void ReverbComponent::SetReverb(float reverb)
	{
		GM_ASSERT_RETURN(reverb >= 0.f && reverb <= _maxReverb, "Reverb는 0 이상 Max Reverb 이하여야 합니다. reverb=%f, max=%f", reverb, _maxReverb);
		if (_reverb == reverb)
			return;

		const float previousReverb = _reverb;
		_reverb = reverb;
		PublishReverbChanged(previousReverb);
	}

	void ReverbComponent::HandleAttackHit(const HitEvent& event)
	{
		if (event.damageResult.state == DamageState::Applied)
			AddReverb(_reverbGainPerHit);
	}

	void ReverbComponent::SetReverbGainPerHit(float reverbGainPerHit)
	{
		GM_ASSERT_RETURN(reverbGainPerHit >= 0.f, "타격당 Reverb 획득량은 0 이상이어야 합니다.");
		_reverbGainPerHit = reverbGainPerHit;
	}

	void ReverbComponent::PublishReverbChanged(float previousReverb)
	{
		ReverbChangedEvent event{};
		event.previousReverb = previousReverb;
		event.currentReverb = _reverb;
		event.maxReverb = _maxReverb;
		OnReverbChanged.Publish(event);
	}
}
