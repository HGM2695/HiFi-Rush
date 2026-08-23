#pragma once

#include "Event.h"
#include "GameplayScene.h"

namespace gm
{
	class AttackGuideWidget;
	class RhythmTutorialWidget;
	class PlayerControlComponent;
	struct AttackGuideCompletedEvent;
	struct RhythmTutorialCompletedEvent;
	struct RhythmTutorialInputPhaseEvent;
	struct DialogBranchRequestedEvent;
	struct DialogFinishedEvent;

	class TutorialScene : public GameplayScene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnExit() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;

	private:
		enum class TutorialPhase
		{
			IntroDialog,
			WeakAttackGuide,
			WeakRhythmDialog,
			StrongAttackIntroDialog,
			StrongAttackGuide,
			StrongRhythmDialog,
			Completed,
		};

		void InitializeTutorialFlow();
		void HandleDialogFinished(const DialogFinishedEvent& event);
		void HandleDialogBranchRequested(const DialogBranchRequestedEvent& event);
		void HandleAttackGuideCompleted(const AttackGuideCompletedEvent& event);
		void HandleRhythmTutorialInputPhase(const RhythmTutorialInputPhaseEvent& event);
		void HandleRhythmTutorialCompleted(const RhythmTutorialCompletedEvent& event);
		void CompleteTutorial();
		void ScheduleDialog(const wchar_t* sequenceId);
		void PlayScheduledDialog();
		void PlayScheduledRoadUpBGM();
		void InitializeSubObject();
		void InitializeStaticMeshTest();
		void InitializeSkeletalMeshTest();

		AttackGuideWidget*		_attackGuideWidget = nullptr;
		RhythmTutorialWidget*	_rhythmTutorialWidget = nullptr;
		PlayerControlComponent*	_playerControlComponent = nullptr;
		EventConnection			_dialogFinishedConnection{};
		EventConnection			_dialogBranchRequestedConnection{};
		EventConnection			_attackGuideCompletedConnection{};
		EventConnection			_rhythmTutorialInputPhaseConnection{};
		EventConnection			_rhythmTutorialCompletedConnection{};
		const wchar_t*			_pendingDialogSequenceId = nullptr;
		float					_pendingDialogStartBeat = 0.f;
		float					_roadUpBGMStartBeat = 0.f;
		bool					_isRoadUpBGMQueued = false;
		TutorialPhase			_tutorialPhase = TutorialPhase::IntroDialog;
	};
}
