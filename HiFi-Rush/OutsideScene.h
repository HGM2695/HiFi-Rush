#pragma once

#include "Event.h"
#include "GameplayScene.h"
#include "TriggerBinding.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

namespace gm
{
	struct DialogFinishedEvent;
	struct MonsterDeathAnimationCompletedEvent;
	struct MonsterSpawnResult;
	struct TriggerEvent;

	class OutsideScene : public GameplayScene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnExit() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;

	private:
		enum class EncounterPhase
		{
			Intro,
			FirstWave,
			SecondWave,
			Completed,
		};

	private:
		bool InitializeEncounterMonsters(const std::vector<MonsterSpawnResult>& monsterSpawnResults);
		void HandleDialogFinished(const DialogFinishedEvent& event);
		void StartShuffleTutorial();
		void HandleMonsterDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent& event);
		void StartEncounter();
		bool TrackWaveMonsters(const std::vector<WeakGameObjectPtr>& monsters);
		void DisconnectWaveDeathAnimationEvents();
		void ScheduleSecondWave();
		void CompleteEncounter();
		void UpdateEncounter();
		void HandleTrigger(const TriggerEvent& event);

	private:
		std::vector<WeakGameObjectPtr>	_firstWaveMonsters{};
		std::vector<WeakGameObjectPtr>	_secondWaveMonsters{};
		std::vector<std::unique_ptr<EventConnection>> _waveDeathAnimationConnections{};
		EventConnection					_dialogFinishedConnection{};
		EventConnection					_triggerConnection{};
		TriggerBinding					_shuffleDialogTriggerBinding{};
		std::optional<float>			_triggerSoundBeat{};
		std::array<float, 4>			_panelSoundBeats{};
		uint32							_nextPanelSoundIndex = 4;
		uint32							_remainingWaveMonsterCount = 0;
		EncounterPhase					_encounterPhase = EncounterPhase::Intro;
	};
}
