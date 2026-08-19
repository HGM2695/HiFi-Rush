#pragma once

#include "RhythmTutorialTypes.h"
#include "Event.h"
#include "RhythmInputJudge.h"
#include "UserWidget.h"

#include <array>

namespace gm
{
	class BeatSystem;
	class ChiStateMachineComponent;
	class Image;

	struct RhythmTutorialInputPhaseEvent final : EventType
	{
		RhythmTutorialType	type = RhythmTutorialType::Count;
		bool				isActive = false;
	};

	struct RhythmTutorialCompletedEvent final : EventType
	{
		RhythmTutorialType	type = RhythmTutorialType::Count;
		RhythmJudgeGrade	grade = RhythmJudgeGrade::OffBeat;
	};

	class RhythmTutorialWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"RhythmTutorialRoot";

		RhythmTutorialWidget(const BeatSystem& beatSystem, const RhythmJudge& rhythmJudge, ChiStateMachineComponent& stateMachine);

		void Play(RhythmTutorialType type);
		void Hide();
		bool IsPlaying() const;

		EventPublisher<RhythmTutorialWidget, RhythmTutorialInputPhaseEvent>	OnInputPhaseChanged;
		EventPublisher<RhythmTutorialWidget, RhythmTutorialCompletedEvent>	OnCompleted;

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		enum class PlaybackState
		{
			Hidden,
			Waiting,
			Demonstration,
			Input,
			Result,
		};

		struct FeedbackVisual
		{
			Image*	thunder = nullptr;
			Image*	result = nullptr;
			float	startBeat = 0.f;
			bool	active = false;
			bool	hasThunder = false;
		};

		void			HandleRhythmActionStarted(const RhythmJudgeResult& result);
		void			ConfigureTypeVisuals();
		void			ResetVisuals();
		void			BeginInputPhase();
		void			EndInputPhase();
		void			CheckMissedInput(float currentBeat);
		void			FailTutorial();
		void			AddInputFeedback(uint32 inputIndex, RhythmJudgeGrade grade, float inputX);
		void			AddMissingInputFeedback(uint32 inputIndex);
		void			UpdateFeedback(float currentBeat);
		void			UpdateBackgroundPulse(float currentBeat, float elapsedBeat);
		void			UpdateTimeline(float elapsedBeat);
		void			UpdateMarkerHighlights(float elapsedBeat);
		void			PlayCueSounds(float elapsedBeat);
		void			PlayCue(const wchar_t* resourceKey);
		void			ShowFinalResult();
		void			Complete();
		float			CalculateInputX(const RhythmJudgeResult& result) const;
		RhythmInputType GetExpectedInputType() const;

		const BeatSystem&				_beatSystem;
		const RhythmJudge&				_rhythmJudge;
		ChiStateMachineComponent&		_stateMachine;
		EventConnection					_rhythmActionConnection{};
		Image*							_topBackground = nullptr;
		Image*							_bottomBackground = nullptr;
		Image*							_topBackAccent = nullptr;
		Image*							_bottomBackAccent = nullptr;
		Image*							_topFrontAccent = nullptr;
		Image*							_bottomFrontAccent = nullptr;
		std::array<Image*, 12>			_circleMarkers{};
		std::array<Image*, 12>			_circleHighlights{};
		std::array<Image*, 4>			_lineMarkers{};
		std::array<Image*, 4>			_lineHighlights{};
		std::array<Image*, 4>			_mouseButtons{};
		std::array<FeedbackVisual, 4>	_feedbackVisuals{};
		Image*							_timeline = nullptr;
		Image*							_finalResult = nullptr;
		std::array<float, 4>			_targetBeatOffsets{};
		std::array<float, 4>			_targetCentersX{};
		RhythmTutorialType				_activeType = RhythmTutorialType::Count;
		PlaybackState					_playbackState = PlaybackState::Hidden;
		RhythmJudgeGrade				_resultGrade = RhythmJudgeGrade::Perfect;
		float							_startBeat = 0.f;
		float							_resultHideBeat = 0.f;
		uint32							_targetCount = 0;
		uint32							_inputCount = 0;
		uint32							_nextCueIndex = 0;
		bool							_isInputPhaseActive = false;
		bool							_hasFailed = false;
		bool							_hasCompleted = false;
	};
}
