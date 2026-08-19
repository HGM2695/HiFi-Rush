#pragma once

#include "Event.h"
#include "GameplayScene.h"

#include <memory>
#include <optional>
#include <vector>

namespace gm
{
	struct DialogFinishedEvent;
	struct MonsterDeathAnimationCompletedEvent;
	struct MonsterSpawnResult;

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
		void HandleMonsterDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent& event);
		void StartEncounter();
		bool TrackWaveMonsters(const std::vector<WeakGameObjectPtr>& monsters);
		void DisconnectWaveDeathAnimationEvents();
		void ScheduleSecondWave();
		void CompleteEncounter();
		void UpdateEncounter();

	private:
		std::vector<WeakGameObjectPtr>	_firstWaveMonsters{};
		std::vector<WeakGameObjectPtr>	_secondWaveMonsters{};
		std::vector<std::unique_ptr<EventConnection>> _waveDeathAnimationConnections{};
		EventConnection					_dialogFinishedConnection{};
		std::optional<float>			_triggerSoundBeat{};
		uint32							_remainingWaveMonsterCount = 0;
		EncounterPhase					_encounterPhase = EncounterPhase::Intro;
	};
}
