#include "WidgetTween.h"
#include "Widget.h"

#include <algorithm>
#include <cmath>

	namespace gm
{
	WidgetTween::WidgetTween(const WidgetTweenDesc& desc)
		: _startDelay(std::max(0.f, desc.startDelay)),
		_forwardDuration(std::max(0.f, desc.forwardDuration)),
		_holdDuration(std::max(0.f, desc.holdDuration)),
		_backwardDuration(std::max(0.f, desc.backwardDuration)),
		_repeatDelay(std::max(0.f, desc.repeatDelay)),
		_ease(desc.ease),
		_isRepeat(desc.repeat)
	{
		if (_startDelay + _forwardDuration + _holdDuration + _backwardDuration + _repeatDelay <= 0.f)
			_isRepeat = false;
	}

	void WidgetTween::Start(Widget& widget)
	{
		_elapsed = 0.f;
		_isCompleted = false;

		if (_isRepeat == false && _startDelay + _forwardDuration + _holdDuration + _backwardDuration <= 0.f)
		{
			Apply(widget, 1.f);
			_isCompleted = true;
		}
		else
		{
			Apply(widget, 0.f);
		}
	}

	void WidgetTween::Tick(Widget& widget, float deltaTime)
	{
		if (_isCompleted)
			return;

		_elapsed += std::max(0.f, deltaTime);
		float playTime = _elapsed;
		if (_isRepeat)
		{
			const float cycleDuration = _startDelay + _forwardDuration + _holdDuration + _backwardDuration + _repeatDelay;
			playTime = std::fmod(_elapsed, cycleDuration);
		}

		if (playTime < _startDelay)
		{
			Apply(widget, 0.f);
			return;
		}

		playTime -= _startDelay;

		if (_forwardDuration > 0.f && playTime < _forwardDuration)
		{
			const float ratio = std::clamp(playTime / _forwardDuration, 0.f, 1.f);
			Apply(widget, EvaluateEase(ratio));
			return;
		}

		playTime -= _forwardDuration;

		if (playTime < _holdDuration)
		{
			Apply(widget, 1.f);
			return;
		}

		playTime -= _holdDuration;

		if (_backwardDuration > 0.f)
		{
			if (playTime < _backwardDuration)
			{
				const float ratio = std::clamp(playTime / _backwardDuration, 0.f, 1.f);
				Apply(widget, std::clamp(EvaluateEase(1.f - ratio), 0.f, 1.f));
				return;
			}

			Apply(widget, 0.f);
			if (_isRepeat == false)
				_isCompleted = true;
			return;
		}

		if (_isRepeat)
		{
			Apply(widget, 0.f);
			return;
		}

		Apply(widget, 1.f);
		_isCompleted = true;
	}

	float WidgetTween::EvaluateEase(float ratio) const
	{
		switch (_ease)
		{
		case WidgetTweenEase::Linear:
			return ratio;
		case WidgetTweenEase::OutBack:
		{
			constexpr float BackAmount = 1.70158f;
			constexpr float BackScale = BackAmount + 1.f;
			const float offsetRatio = ratio - 1.f;
			return 1.f + BackScale * offsetRatio * offsetRatio * offsetRatio + BackAmount * offsetRatio * offsetRatio;
		}
		default:
			return ratio;
		}
	}

	WidgetSizeTween::WidgetSizeTween(const WidgetSizeTweenDesc& desc)
		: WidgetTween(desc),
		_from(desc.from),
		_to(desc.to)
	{
	}

	void WidgetSizeTween::Apply(Widget& widget, float ratio)
	{
		widget.SetSize(_from + (_to - _from) * ratio);
	}
}
