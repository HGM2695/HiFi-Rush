#include "BeatHitWidget.h"

#include "AudioStatics.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "BeatHitTypes.h"
#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "Image.h"
#include "HiFiRushAudio.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		constexpr Vector2 ScreenCenter{ 800.f, 450.f };
		constexpr Vector2 InsideCircleSize{ 170.f, 170.f };
		constexpr Vector2 InsideGlowSize{ 212.5f, 212.5f };
		constexpr Vector2 OutsideCircleStartSize{ 317.5f, 317.5f };
		constexpr Vector2 OutsideCircleTargetSize = InsideCircleSize;
		constexpr Vector2 OutsideGlowStartSize{ 362.25f, 362.25f };
		constexpr Vector2 OutsideGlowTargetSize{ 191.25f, 191.25f };
		constexpr Vector2 LightCircleSize{ 1250.f, 1197.5f };
		constexpr Vector2 LightLineSize{ 1011.25f, 1000.f };
		constexpr Vector2 ThornSize{ 1800.f, 900.f };

		constexpr Color ReadyColor{ 95.f / 255.f, 191.f / 255.f, 71.f / 255.f, 1.f };
		constexpr Color MissColor{ 228.5f / 255.f, 0.f, 0.f, 1.f };
		constexpr Color SuccessColor{ 0.5f, 0.5f, 0.5f, 1.f };

		constexpr float InsideGlowOpacity = 0.7f;
		constexpr float OutsideGlowOpacity = 0.65f;
		constexpr float GlowPulseSharpness = 4.f;
		constexpr float ResultDurationBeats = 1.f;
	}

	BeatHitWidget::BeatHitWidget(const BeatSystem& beatSystem, ChiStateMachineComponent& stateMachine)
		: _beatSystem(beatSystem), _stateMachine(stateMachine)
	{
	}

	void BeatHitWidget::Play(float targetBeat, float approachDurationBeats)
	{
		GM_ASSERT_RETURN(approachDurationBeats > 0.f, "Beat Hit 접근 시간은 0 Beat보다 길어야 합니다.");

		_targetBeat = targetBeat;
		_approachDurationBeats = approachDurationBeats;
		_approachStartBeat = _targetBeat - _approachDurationBeats;
		_resultStartBeat.reset();
		_state = PresentationState::Approaching;
		ResetVisuals();
		SetVisible(true);

		if (_beatSystem.HasPlaybackTime())
		{
			const float currentBeat = _beatSystem.GetCurrentBeat();
			const float ratio = std::clamp((currentBeat - _approachStartBeat) / _approachDurationBeats, 0.f, 1.f);
			ApplyApproach(ratio);
			UpdateApproachGlows(currentBeat);
		}
		else
		{
			ApplyApproach(0.f);
			_insideGlow->SetOpacity(0.f);
			_outsideGlow->SetOpacity(0.f);
		}
	}

	void BeatHitWidget::ShowSuccess()
	{
		if (_state == PresentationState::Hidden)
			return;

		BeginResult(PresentationState::Success);
		PlaySound2D(HiFiRushSound::BeatHitSuccess, 0.3f);
	}

	void BeatHitWidget::ShowMiss()
	{
		if (_state == PresentationState::Hidden)
			return;

		BeginResult(PresentationState::Miss);
	}

	void BeatHitWidget::Hide()
	{
		_state = PresentationState::Hidden;
		_resultStartBeat.reset();
		SetVisible(false);
	}

	bool BeatHitWidget::IsPlaying() const
	{
		return _state != PresentationState::Hidden;
	}

	std::unique_ptr<Widget> BeatHitWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);

		_insideGlow = root->AddNamedChild<Image>(L"BeatHitInsideGlow", L"T_BH_circle_inside_glow");
		_insideGlow->SetGeometry(ScreenCenter, InsideGlowSize);
		_insideCircle = root->AddNamedChild<Image>(L"BeatHitInsideCircle", L"T_BH_circle_inside");
		_insideCircle->SetGeometry(ScreenCenter, InsideCircleSize);

		_outsideGlow = root->AddNamedChild<Image>(L"BeatHitOutsideGlow", L"T_BH_circle_outside_glow");
		_outsideGlow->SetGeometry(ScreenCenter, OutsideGlowStartSize);
		_outsideCircle = root->AddNamedChild<Image>(L"BeatHitOutsideCircle", L"T_BH_circle_outside");
		_outsideCircle->SetGeometry(ScreenCenter, OutsideCircleStartSize);

		_lightCircle = root->AddNamedChild<Image>(L"BeatHitLightCircle", L"T_BH_light");
		_lightCircle->SetGeometry(ScreenCenter, LightCircleSize);
		_lightLine = root->AddNamedChild<Image>(L"BeatHitLightLine", L"T_BH_light_small");
		_lightLine->SetGeometry(ScreenCenter, LightLineSize);
		_thorn = root->AddNamedChild<Image>(L"BeatHitThorn", L"Thorn");
		_thorn->SetGeometry(ScreenCenter, ThornSize);

		return root;
	}

	void BeatHitWidget::OnInitialize()
	{
		_stateMachine.OnBeatHitStarted.Subscribe(_beatHitStartedConnection,
			[this](const BeatHitStartedEvent& event)
			{
				Play(event.targetBeat, event.approachDurationBeats);
			});
		_stateMachine.OnBeatHitResult.Subscribe(_beatHitResultConnection,
			[this](const BeatHitResultEvent& event)
			{
				switch (event.result)
				{
				case BeatHitResult::Success:
					ShowSuccess();
					break;
				case BeatHitResult::Miss:
					ShowMiss();
					break;
				case BeatHitResult::Cancelled:
					Hide();
					break;
				}
			});
		Hide();
	}

	void BeatHitWidget::OnTick(float)
	{
		if (_beatSystem.HasPlaybackTime() == false)
			return;

		const float currentBeat = _beatSystem.GetCurrentBeat();
		if (_state == PresentationState::Approaching)
		{
			const float ratio = std::clamp((currentBeat - _approachStartBeat) / _approachDurationBeats, 0.f, 1.f);
			ApplyApproach(ratio);
			UpdateApproachGlows(currentBeat);
			return;
		}

		if (_state != PresentationState::Success && _state != PresentationState::Miss)
			return;

		if (_resultStartBeat.has_value() == false)
			_resultStartBeat = currentBeat;

		const float elapsedBeats = currentBeat - _resultStartBeat.value();
		if (elapsedBeats >= ResultDurationBeats)
		{
			Hide();
			return;
		}

		ApplyResultOpacity(1.f - std::clamp(elapsedBeats / ResultDurationBeats, 0.f, 1.f));
	}

	void BeatHitWidget::ResetVisuals()
	{
		_insideCircle->SetVisible(true);
		_insideGlow->SetVisible(true);
		_outsideGlow->SetVisible(true);
		_outsideCircle->SetVisible(true);
		_lightCircle->SetVisible(false);
		_lightLine->SetVisible(false);
		_thorn->SetVisible(false);

		_insideCircle->SetColorBlend(ReadyColor, 1.f);
		_insideGlow->SetColorBlend(ReadyColor, 1.f);
		_outsideGlow->SetColorBlend(MissColor, 1.f);
		_outsideCircle->SetColorBlend(MissColor, 1.f);
		_insideGlow->SetOpacity(0.f);
		_outsideGlow->SetOpacity(0.f);
		ApplyResultOpacity(1.f);
	}

	void BeatHitWidget::ApplyApproach(float ratio)
	{
		_outsideGlow->SetSize(OutsideGlowStartSize + (OutsideGlowTargetSize - OutsideGlowStartSize) * ratio);
		_outsideCircle->SetSize(OutsideCircleStartSize + (OutsideCircleTargetSize - OutsideCircleStartSize) * ratio);
	}

	void BeatHitWidget::UpdateApproachGlows(float currentBeat)
	{
		const float boundaryPulse = BeatMath::EvaluateBeatBoundaryPulse(currentBeat, 1.f);
		const float pulse = std::pow(boundaryPulse, GlowPulseSharpness);
		_insideGlow->SetOpacity(pulse * InsideGlowOpacity);
		_outsideGlow->SetOpacity(pulse * OutsideGlowOpacity);
	}

	void BeatHitWidget::ApplyResultOpacity(float opacity)
	{
		_insideCircle->SetOpacity(opacity);
		_outsideCircle->SetOpacity(opacity);
		_lightCircle->SetOpacity(opacity);
		_lightLine->SetOpacity(opacity);
		_thorn->SetOpacity(opacity);
	}

	void BeatHitWidget::BeginResult(PresentationState resultState)
	{
		_state = resultState;
		_resultStartBeat = _beatSystem.HasPlaybackTime() ? std::optional<float>{ _beatSystem.GetCurrentBeat() } : std::nullopt;
		ApplyApproach(1.f);

		const bool succeeded = _state == PresentationState::Success;
		_insideCircle->SetColorBlend(succeeded ? SuccessColor : MissColor, 1.f);
		_insideGlow->SetColorBlend(succeeded ? SuccessColor : MissColor, 1.f);
		_insideGlow->SetVisible(false);
		_outsideGlow->SetVisible(false);
		_outsideCircle->SetVisible(succeeded == false);
		_lightCircle->SetVisible(succeeded);
		_lightLine->SetVisible(succeeded);
		_thorn->SetVisible(succeeded);
		ApplyResultOpacity(1.f);
	}
}
