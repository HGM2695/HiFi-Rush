#include "RhythmTutorialWidget.h"

#include "Application.h"
#include "AudioSystem.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "Image.h"
#include "Resources.h"
#include "SoundWave.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float DemonstrationDurationBeats = 8.2f;
		constexpr float BottomTimelineStartBeat = 8.f;
		constexpr float TutorialDurationBeats = 14.f;
		constexpr float ResultDisplayDurationBeats = 0.75f;
		constexpr float TimelinePixelsPerBeat = 88.75f;
		constexpr float CuePlaybackLeadBeats = 0.5f;

		struct TutorialCue
		{
			float			beat = 0.f;
			const wchar_t*	resourceKey = nullptr;
			bool			playPulse = false;
		};

		constexpr std::array WeakTutorialCues
		{
			TutorialCue{ 0.f, L"TutorialCue.Go", false },
			TutorialCue{ 1.f, L"TutorialCue.Count", true },
			TutorialCue{ 2.f, L"TutorialCue.Count", true },
			TutorialCue{ 3.f, L"TutorialCue.Count", true },
			TutorialCue{ 4.f, L"TutorialCue.Count", true },
			TutorialCue{ 5.f, L"TutorialCue.One", false },
			TutorialCue{ 6.f, L"TutorialCue.Two", false },
			TutorialCue{ 7.f, L"TutorialCue.Three", false },
			TutorialCue{ 8.f, L"TutorialCue.Go", false },
		};

		constexpr std::array StrongTutorialCues
		{
			TutorialCue{ 0.f, L"TutorialCue.Go", false },
			TutorialCue{ 1.f, L"TutorialCue.Count", true },
			TutorialCue{ 3.f, L"TutorialCue.Count", true },
			TutorialCue{ 5.f, L"TutorialCue.Count", true },
			TutorialCue{ 7.f, L"TutorialCue.Go", false },
		};

		constexpr Vector2 TopBackgroundCenter{ 801.25f, 103.75f };
		constexpr Vector2 TopBackgroundSize{ 750.f, 132.5f };
		constexpr Vector2 BottomBackgroundCenter{ 801.25f, 290.625f };
		constexpr Vector2 BottomBackgroundSize{ 750.f, 118.75f };
		constexpr Vector2 TopBackAccentCenter{ 784.375f, 111.25f };
		constexpr Vector2 TopBackAccentSize{ 876.25f, 210.f };
		constexpr Vector2 BottomBackAccentCenter{ 785.f, 292.5f };
		constexpr Vector2 BottomBackAccentSize{ 877.5f, 207.5f };
		constexpr Vector2 TopFrontAccentCenter{ 803.125f, 101.875f };
		constexpr Vector2 TopFrontAccentSize{ 781.25f, 156.25f };
		constexpr Vector2 BottomFrontAccentCenter{ 799.375f, 283.125f };
		constexpr Vector2 BottomFrontAccentSize{ 781.25f, 156.25f };
		constexpr Vector2 CircleSize{ 32.5f, 30.f };
		constexpr Vector2 CircleHighlightSize{ 42.5f, 40.f };
		constexpr Vector2 LineSize{ 15.f, 65.f };
		constexpr Vector2 LineHighlightSize{ 17.5f, 76.25f };
		constexpr Vector2 MouseButtonSize{ 30.f, 41.25f };
		constexpr Vector2 TimelineSize{ 12.5f, 122.5f };
		constexpr Vector2 FeedbackResultSize{ 75.f, 30.f };
		constexpr Vector2 FinalResultCenter{ 1168.125f, 468.75f };
		constexpr Vector2 FinalResultSize{ 298.75f, 125.f };
		constexpr float TopMarkerY = 113.75f;
		constexpr float BottomCircleY = 288.75f;
		constexpr float BottomLineY = 290.f;
		constexpr float MouseButtonY = 289.375f;
		constexpr float TimelineStartX = 426.25f;
		constexpr float TopTimelineY = 112.5f;
		constexpr float BottomTimelineY = 292.5f;
		constexpr float FeedbackResultY = 327.5f;
		constexpr float FeedbackThunderY = 247.5f;
		constexpr std::array<float, 6> CircleCentersX{ 603.75f, 692.5f, 781.25f, 958.75f, 1047.5f, 1136.25f };
		constexpr std::array<float, 2> LineCentersX{ 515.f, 870.f };
		constexpr Color HighlightColor{ 242.f / 255.f, 139.f / 255.f, 51.f / 255.f, 1.f };
		constexpr Color PerfectColor{ 129.f / 255.f, 1.f, 120.f / 255.f, 1.f };
		constexpr Color MissColor{ 1.f, 50.f / 255.f, 49.f / 255.f, 1.f };

		const wchar_t* GetFeedbackTextureKey(RhythmJudgeGrade grade)
		{
			switch (grade)
			{
			case RhythmJudgeGrade::Perfect:
				return L"T_Word_CmnResult_Perfect";
			case RhythmJudgeGrade::Good:
				return L"T_Word_CmnResult_Good";
			case RhythmJudgeGrade::OffBeat:
				return L"T_Word_CmnResult_Miss";
			default:
				return L"T_Word_CmnResult_Miss";
			}
		}

		const wchar_t* GetFinalResultTextureKey(RhythmJudgeGrade grade)
		{
			switch (grade)
			{
			case RhythmJudgeGrade::Perfect:
				return L"T_Word_Tut_Result_Perfect";
			case RhythmJudgeGrade::Good:
				return L"T_Word_Tut_Result_Good";
			case RhythmJudgeGrade::OffBeat:
				return L"T_Word_Tut_Result_Miss";
			default:
				return L"T_Word_Tut_Result_Miss";
			}
		}
	}

	RhythmTutorialWidget::RhythmTutorialWidget(const BeatSystem& beatSystem, const RhythmJudge& rhythmJudge, ChiStateMachineComponent& stateMachine)
		: _beatSystem(beatSystem), _rhythmJudge(rhythmJudge), _stateMachine(stateMachine)
	{
	}

	void RhythmTutorialWidget::Play(RhythmTutorialType type)
	{
		GM_ASSERT_RETURN(type < RhythmTutorialType::Count, "지원하지 않는 Rhythm Tutorial Type입니다.");

		_activeType = type;
		const int64 currentBeatIndex = static_cast<int64>(std::floor(_beatSystem.GetCurrentBeat()));
		_startBeat = static_cast<float>(currentBeatIndex + 9 - currentBeatIndex % 8);
		_playbackState = PlaybackState::Waiting;
		_resultGrade = RhythmJudgeGrade::Perfect;
		_resultHideBeat = 0.f;
		_inputCount = 0;
		_nextCueIndex = 0;
		_isInputPhaseActive = false;
		_hasFailed = false;
		_hasCompleted = false;
		ResetVisuals();
		ConfigureTypeVisuals();
		SetVisible(true);
	}

	void RhythmTutorialWidget::Hide()
	{
		_playbackState = PlaybackState::Hidden;
		_activeType = RhythmTutorialType::Count;
		_isInputPhaseActive = false;
		SetVisible(false);
	}

	bool RhythmTutorialWidget::IsPlaying() const
	{
		return _playbackState != PlaybackState::Hidden;
	}

	std::unique_ptr<Widget> RhythmTutorialWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_topBackAccent = root->AddNamedChild<Image>(L"RhythmTutorialTopBackAccent", L"T_tut_rhythm_bg_top_anim");
		_bottomBackAccent = root->AddNamedChild<Image>(L"RhythmTutorialBottomBackAccent", L"T_tut_rhythm_bg_bottom_anim");
		_topBackAccent->SetGeometry(TopBackAccentCenter, TopBackAccentSize);
		_bottomBackAccent->SetGeometry(BottomBackAccentCenter, BottomBackAccentSize);
		_topBackground = root->AddNamedChild<Image>(L"RhythmTutorialTopBackground", L"T_tut_rhythm_bg_top");
		_bottomBackground = root->AddNamedChild<Image>(L"RhythmTutorialBottomBackground", L"T_tut_rhythm_bg_bottom");
		_topBackground->SetGeometry(TopBackgroundCenter, TopBackgroundSize);
		_bottomBackground->SetGeometry(BottomBackgroundCenter, BottomBackgroundSize);

		for (uint32 rowIndex = 0; rowIndex < 2; ++rowIndex)
		{
			const float circleY = rowIndex == 0 ? TopMarkerY : BottomCircleY;
			const float lineY = rowIndex == 0 ? TopMarkerY : BottomLineY;
			for (uint32 markerIndex = 0; markerIndex < CircleCentersX.size(); ++markerIndex)
			{
				const uint32 index = rowIndex * static_cast<uint32>(CircleCentersX.size()) + markerIndex;
				_circleHighlights[index] = root->AddNamedChild<Image>(L"RhythmTutorialCircleHighlight" + std::to_wstring(index), L"T_tut_rhythm_circle_bg");
				_circleHighlights[index]->SetGeometry({ CircleCentersX[markerIndex], circleY }, CircleHighlightSize);
				_circleHighlights[index]->SetColorBlend(HighlightColor, 1.f);
				_circleMarkers[index] = root->AddNamedChild<Image>(L"RhythmTutorialCircle" + std::to_wstring(index), L"T_tut_rhythm_circle");
				_circleMarkers[index]->SetGeometry({ CircleCentersX[markerIndex], circleY }, CircleSize);
			}

			for (uint32 markerIndex = 0; markerIndex < LineCentersX.size(); ++markerIndex)
			{
				const uint32 index = rowIndex * static_cast<uint32>(LineCentersX.size()) + markerIndex;
				_lineHighlights[index] = root->AddNamedChild<Image>(L"RhythmTutorialLineHighlight" + std::to_wstring(index), L"T_tut_rhythm_vert_line_bg");
				_lineHighlights[index]->SetGeometry({ LineCentersX[markerIndex], lineY }, LineHighlightSize);
				_lineHighlights[index]->SetColorBlend(HighlightColor, 1.f);
				_lineMarkers[index] = root->AddNamedChild<Image>(L"RhythmTutorialLine" + std::to_wstring(index), L"T_tut_rhythm_vert_line");
				_lineMarkers[index]->SetGeometry({ LineCentersX[markerIndex], lineY }, LineSize);
			}
		}

		for (uint32 inputIndex = 0; inputIndex < _mouseButtons.size(); ++inputIndex)
		{
			_mouseButtons[inputIndex] = root->AddNamedChild<Image>(L"RhythmTutorialMouseButton" + std::to_wstring(inputIndex), L"MouseButton");
			_mouseButtons[inputIndex]->SetSize(MouseButtonSize);
			FeedbackVisual& feedback = _feedbackVisuals[inputIndex];
			feedback.thunder = root->AddNamedChild<Image>(L"RhythmTutorialThunder" + std::to_wstring(inputIndex), L"T_tut_rhythm_bar_timing");
			feedback.result = root->AddNamedChild<Image>(L"RhythmTutorialResult" + std::to_wstring(inputIndex), L"T_Word_CmnResult_Perfect");
		}

		_timeline = root->AddNamedChild<Image>(L"RhythmTutorialTimeline", L"T_tut_rhythm_bar_timeline");
		_timeline->SetSize(TimelineSize);
		_topFrontAccent = root->AddNamedChild<Image>(L"RhythmTutorialTopFrontAccent", L"T_tut_rhythm_bg_detail_anim");
		_bottomFrontAccent = root->AddNamedChild<Image>(L"RhythmTutorialBottomFrontAccent", L"T_tut_rhythm_bg_detail_anim");
		_topFrontAccent->SetGeometry(TopFrontAccentCenter, TopFrontAccentSize);
		_bottomFrontAccent->SetGeometry(BottomFrontAccentCenter, BottomFrontAccentSize);
		_finalResult = root->AddNamedChild<Image>(L"RhythmTutorialFinalResult", L"T_Word_Tut_Result_Perfect");
		_finalResult->SetGeometry(FinalResultCenter, FinalResultSize);
		return root;
	}

	void RhythmTutorialWidget::OnInitialize()
	{
		_stateMachine.OnRhythmActionStarted.Subscribe(_rhythmActionConnection,
			[this](const RhythmJudgeResult& result)
			{
				HandleRhythmActionStarted(result);
			});
		Hide();
	}

	void RhythmTutorialWidget::OnTick(float)
	{
		const float currentBeat = _beatSystem.GetCurrentBeat();
		if (_playbackState == PlaybackState::Result)
		{
			UpdateFeedback(currentBeat);
			if (currentBeat >= _resultHideBeat)
				Hide();
			return;
		}

		const float elapsedBeat = currentBeat - _startBeat;
		PlayCueSounds(elapsedBeat);
		if (elapsedBeat < 0.f)
			return;

		if (_playbackState == PlaybackState::Waiting)
			_playbackState = PlaybackState::Demonstration;

		UpdateBackgroundPulse(currentBeat, elapsedBeat);
		UpdateTimeline(elapsedBeat);
		UpdateMarkerHighlights(elapsedBeat);
		UpdateFeedback(currentBeat);

		if (_playbackState == PlaybackState::Demonstration && elapsedBeat >= DemonstrationDurationBeats)
			BeginInputPhase();
		if (_playbackState == PlaybackState::Input)
			CheckMissedInput();

		if (elapsedBeat >= TutorialDurationBeats && _hasCompleted == false)
			Complete();
	}

	void RhythmTutorialWidget::HandleRhythmActionStarted(const RhythmJudgeResult& result)
	{
		if (_isInputPhaseActive == false || _hasFailed || result.type != GetExpectedInputType())
			return;

		const float expectedBeat = _startBeat + _targetBeatOffsets[_inputCount];
		const bool isExpectedBeat = result.judgedBeatIndex == static_cast<int64>(std::round(expectedBeat));
		if (isExpectedBeat == false || result.judgeGrade == RhythmJudgeGrade::OffBeat)
		{
			AddInputFeedback(_inputCount, RhythmJudgeGrade::OffBeat, CalculateInputX(result));
			++_inputCount;
			FailTutorial();
			return;
		}

		AddInputFeedback(_inputCount, result.judgeGrade, CalculateInputX(result));
		if (result.judgeGrade == RhythmJudgeGrade::Good)
			_resultGrade = RhythmJudgeGrade::Good;
		++_inputCount;
		if (_inputCount >= _targetCount)
			EndInputPhase();
	}

	void RhythmTutorialWidget::ConfigureTypeVisuals()
	{
		if (_activeType == RhythmTutorialType::Weak)
		{
			_targetCount = 4;
			_targetBeatOffsets = { 9.f, 10.f, 11.f, 12.f };
			_targetCentersX = { LineCentersX[0], CircleCentersX[0], CircleCentersX[1], CircleCentersX[2] };
			_lineMarkers[2]->SetVisible(false);
			_circleMarkers[6]->SetVisible(false);
			_circleMarkers[7]->SetVisible(false);
			_circleMarkers[8]->SetVisible(false);
		}
		else
		{
			_targetCount = 3;
			_targetBeatOffsets = { 9.f, 11.f, 13.f, 0.f };
			_targetCentersX = { LineCentersX[0], CircleCentersX[1], LineCentersX[1], 0.f };
			_lineMarkers[2]->SetVisible(false);
			_lineMarkers[3]->SetVisible(false);
			_circleMarkers[7]->SetVisible(false);
		}

		const wchar_t* mouseTextureKey = _activeType == RhythmTutorialType::Weak ? L"MouseButton" : L"MouseButtonRight";
		for (uint32 inputIndex = 0; inputIndex < _targetCount; ++inputIndex)
		{
			_mouseButtons[inputIndex]->SetTexture(mouseTextureKey);
			_mouseButtons[inputIndex]->SetPosition({ _targetCentersX[inputIndex], MouseButtonY });
			_mouseButtons[inputIndex]->SetVisible(true);
		}
	}

	void RhythmTutorialWidget::ResetVisuals()
	{
		_topBackground->SetVisible(true);
		_bottomBackground->SetVisible(true);
		_topBackAccent->SetVisible(true);
		_bottomBackAccent->SetVisible(false);
		_topFrontAccent->SetVisible(true);
		_bottomFrontAccent->SetVisible(false);
		_topBackAccent->SetSize(TopBackAccentSize);
		_bottomBackAccent->SetSize(BottomBackAccentSize);
		_topFrontAccent->SetSize(TopFrontAccentSize);
		_bottomFrontAccent->SetSize(BottomFrontAccentSize);
		_timeline->SetVisible(false);
		_finalResult->SetVisible(false);

		for (Image* marker : _circleMarkers)
			marker->SetVisible(true);
		for (Image* marker : _lineMarkers)
			marker->SetVisible(true);
		for (Image* highlight : _circleHighlights)
			highlight->SetVisible(false);
		for (Image* highlight : _lineHighlights)
			highlight->SetVisible(false);
		for (Image* mouseButton : _mouseButtons)
			mouseButton->SetVisible(false);
		for (FeedbackVisual& feedback : _feedbackVisuals)
		{
			feedback.active = false;
			feedback.hasThunder = false;
			feedback.thunder->SetVisible(false);
			feedback.result->SetVisible(false);
		}
	}

	void RhythmTutorialWidget::BeginInputPhase()
	{
		_playbackState = PlaybackState::Input;
		_isInputPhaseActive = true;
		RhythmTutorialInputPhaseEvent event{};
		event.type = _activeType;
		event.isActive = true;
		OnInputPhaseChanged.Publish(event);
	}

	void RhythmTutorialWidget::EndInputPhase()
	{
		if (_isInputPhaseActive == false)
			return;

		_isInputPhaseActive = false;
		RhythmTutorialInputPhaseEvent event{};
		event.type = _activeType;
		event.isActive = false;
		OnInputPhaseChanged.Publish(event);
	}

	void RhythmTutorialWidget::CheckMissedInput()
	{
		if (_inputCount >= _targetCount)
			return;

		const float expectedBeat = _startBeat + _targetBeatOffsets[_inputCount];
		if (_rhythmJudge.HasPassedInputDeadline(_beatSystem, expectedBeat))
			FailTutorial();
	}

	void RhythmTutorialWidget::FailTutorial()
	{
		if (_hasFailed)
			return;

		_hasFailed = true;
		_resultGrade = RhythmJudgeGrade::OffBeat;
		for (uint32 inputIndex = _inputCount; inputIndex < _targetCount; ++inputIndex)
			AddMissingInputFeedback(inputIndex);
		EndInputPhase();
	}

	void RhythmTutorialWidget::AddInputFeedback(uint32 inputIndex, RhythmJudgeGrade grade, float inputX)
	{
		GM_ASSERT_RETURN(inputIndex < _feedbackVisuals.size(), "Rhythm Tutorial Feedback Index가 유효하지 않습니다.");

		FeedbackVisual& feedback = _feedbackVisuals[inputIndex];
		feedback.startBeat = _beatSystem.GetCurrentBeat();
		feedback.active = true;
		feedback.hasThunder = true;
		feedback.result->SetTexture(GetFeedbackTextureKey(grade));
		feedback.result->SetGeometry({ inputX, FeedbackResultY }, Vector2{});
		feedback.result->SetVisible(true);
		feedback.thunder->SetGeometry({ inputX, FeedbackThunderY }, { 18.75f, 25.f });
		feedback.thunder->SetColorBlend(grade == RhythmJudgeGrade::OffBeat ? MissColor : grade == RhythmJudgeGrade::Good ? HighlightColor : PerfectColor, 1.f);
		feedback.thunder->SetVisible(true);
	}

	void RhythmTutorialWidget::AddMissingInputFeedback(uint32 inputIndex)
	{
		GM_ASSERT_RETURN(inputIndex < _feedbackVisuals.size(), "Rhythm Tutorial Feedback Index가 유효하지 않습니다.");

		FeedbackVisual& feedback = _feedbackVisuals[inputIndex];
		feedback.startBeat = _beatSystem.GetCurrentBeat();
		feedback.active = true;
		feedback.hasThunder = false;
		feedback.result->SetTexture(GetFeedbackTextureKey(RhythmJudgeGrade::OffBeat));
		feedback.result->SetGeometry({ _targetCentersX[inputIndex], FeedbackResultY }, Vector2{});
		feedback.result->SetVisible(true);
		feedback.thunder->SetVisible(false);
	}

	void RhythmTutorialWidget::UpdateFeedback(float currentBeat)
	{
		for (FeedbackVisual& feedback : _feedbackVisuals)
		{
			if (feedback.active == false)
				continue;

			const float elapsedBeat = std::max(0.f, currentBeat - feedback.startBeat);
			const float resultRatio = std::min(1.f, elapsedBeat * 2.f);
			feedback.result->SetSize(FeedbackResultSize * resultRatio);
			if (feedback.hasThunder == false)
				continue;

			const float thunderRatio = std::min(1.f, elapsedBeat * 3.f);
			const float thunderHeight = 25.f * (1.f + thunderRatio * 3.5f);
			feedback.thunder->SetGeometry({ feedback.thunder->GetPosition().x, FeedbackThunderY + (thunderHeight - 25.f) * 0.5f }, { 18.75f, thunderHeight });
		}
	}

	float RhythmTutorialWidget::CalculateInputX(const RhythmJudgeResult& result) const
	{
		const float inputBeat = _rhythmJudge.GetRawInputBeat(_beatSystem, result);
		const float inputPhaseBeat = inputBeat - _startBeat - BottomTimelineStartBeat;
		return TimelineStartX + inputPhaseBeat * TimelinePixelsPerBeat;
	}

	void RhythmTutorialWidget::UpdateBackgroundPulse(float currentBeat, float elapsedBeat)
	{
		const bool useBottom = elapsedBeat >= DemonstrationDurationBeats;
		_topBackAccent->SetVisible(useBottom == false);
		_topFrontAccent->SetVisible(useBottom == false);
		_bottomBackAccent->SetVisible(useBottom);
		_bottomFrontAccent->SetVisible(useBottom);
		const float pulseScale = 1.f + BeatMath::EvaluateBeatBoundaryPulse(currentBeat, 1.f) * 0.1f;
		if (useBottom)
		{
			_bottomBackAccent->SetSize(BottomBackAccentSize * pulseScale);
			_bottomFrontAccent->SetSize(BottomFrontAccentSize * pulseScale);
		}
		else
		{
			_topBackAccent->SetSize(TopBackAccentSize * pulseScale);
			_topFrontAccent->SetSize(TopFrontAccentSize * pulseScale);
		}
	}

	void RhythmTutorialWidget::UpdateTimeline(float elapsedBeat)
	{
		if (elapsedBeat < 0.f || elapsedBeat > TutorialDurationBeats)
		{
			_timeline->SetVisible(false);
			return;
		}

		const bool useBottom = elapsedBeat >= BottomTimelineStartBeat;
		const float rowBeat = useBottom ? elapsedBeat - BottomTimelineStartBeat : elapsedBeat;
		_timeline->SetPosition({ TimelineStartX + rowBeat * TimelinePixelsPerBeat, useBottom ? BottomTimelineY : TopTimelineY });
		_timeline->SetVisible(true);
	}

	void RhythmTutorialWidget::UpdateMarkerHighlights(float elapsedBeat)
	{
		_lineHighlights[0]->SetVisible(elapsedBeat >= 1.f);
		_circleHighlights[0]->SetVisible(elapsedBeat >= 2.f);
		_circleHighlights[1]->SetVisible(elapsedBeat >= 3.f);
		_circleHighlights[2]->SetVisible(elapsedBeat >= 4.f);
		_lineHighlights[1]->SetVisible(elapsedBeat >= 5.f);
		_circleHighlights[3]->SetVisible(elapsedBeat >= 6.f);
		_circleHighlights[4]->SetVisible(elapsedBeat >= 7.f);
		_circleHighlights[5]->SetVisible(elapsedBeat >= 8.f);
	}

	void RhythmTutorialWidget::PlayCueSounds(float elapsedBeat)
	{
		const TutorialCue* cues = nullptr;
		uint32 cueCount = 0;
		if (_activeType == RhythmTutorialType::Weak)
		{
			cues = WeakTutorialCues.data();
			cueCount = static_cast<uint32>(WeakTutorialCues.size());
		}
		else
		{
			cues = StrongTutorialCues.data();
			cueCount = static_cast<uint32>(StrongTutorialCues.size());
		}

		while (_nextCueIndex < cueCount && elapsedBeat + CuePlaybackLeadBeats >= cues[_nextCueIndex].beat)
		{
			const TutorialCue& cue = cues[_nextCueIndex];
			PlayCue(cue.resourceKey);
			if (cue.playPulse)
				PlayCue(L"TutorialCue.Pulse");
			++_nextCueIndex;
		}
	}

	void RhythmTutorialWidget::PlayCue(const wchar_t* resourceKey)
	{
		const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(resourceKey);
		GM_ASSERT_RETURN(sound, "Rhythm Tutorial Cue가 로드되지 않았습니다. key=%ls", resourceKey);
		APPLICATION.GetAudioSystem().PlaySound2D(*sound, 0.3f);
	}
	void RhythmTutorialWidget::ShowFinalResult()
	{
		_finalResult->SetTexture(GetFinalResultTextureKey(_resultGrade));
		_finalResult->SetVisible(true);
		_timeline->SetVisible(false);
	}

	void RhythmTutorialWidget::Complete()
	{
		if (_inputCount < _targetCount && _hasFailed == false)
			FailTutorial();

		EndInputPhase();
		_hasCompleted = true;
		_playbackState = PlaybackState::Result;
		_resultHideBeat = _beatSystem.GetCurrentBeat() + ResultDisplayDurationBeats;
		ShowFinalResult();
		RhythmTutorialCompletedEvent event{};
		event.type = _activeType;
		event.grade = _resultGrade;
		OnCompleted.Publish(event);
	}

	RhythmInputType RhythmTutorialWidget::GetExpectedInputType() const
	{
		return _activeType == RhythmTutorialType::Weak ? RhythmInputType::WeakAttack : RhythmInputType::StrongAttack;
	}
}
