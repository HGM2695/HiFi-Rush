#pragma once

#include "MathTypes.h"

namespace gm
{
	class Widget;

	enum class WidgetTweenEase
	{
		Linear,
		OutBack,
	};

	struct WidgetTweenDesc
	{
		float				startDelay = 0.f;
		float				forwardDuration = 0.f;
		float				holdDuration = 0.f;
		float				backwardDuration = 0.f;
		float				repeatDelay = 0.f;
		WidgetTweenEase		ease = WidgetTweenEase::Linear;
		bool				repeat = false;
	};

	struct WidgetSizeTweenDesc : WidgetTweenDesc
	{
		Vector2 from{};
		Vector2 to{};
	};

	class WidgetTween
	{
	public:
		explicit WidgetTween(const WidgetTweenDesc& desc);
		virtual ~WidgetTween() = default;

		void Start(Widget& widget);
		void Tick(Widget& widget, float deltaTime);
		bool IsCompleted() const { return _isCompleted; }

	protected:
		virtual void Apply(Widget& widget, float ratio) = 0;

	private:
		float EvaluateEase(float ratio) const;

		float				_elapsed = 0.f;
		float				_startDelay = 0.f;
		float				_forwardDuration = 0.f;
		float				_holdDuration = 0.f;
		float				_backwardDuration = 0.f;
		float				_repeatDelay = 0.f;
		WidgetTweenEase		_ease = WidgetTweenEase::Linear;
		bool				_isRepeat = false;
		bool				_isCompleted = false;
	};

	class WidgetSizeTween final : public WidgetTween
	{
	public:
		explicit WidgetSizeTween(const WidgetSizeTweenDesc& desc);

	protected:
		void Apply(Widget& widget, float ratio) override;

	private:
		Vector2	_from{};
		Vector2	_to{};
	};
}
