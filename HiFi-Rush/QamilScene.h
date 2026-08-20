#pragma once

#include "GameplayScene.h"

#include <memory>
#include <optional>
#include <vector>

namespace gm
{
	struct MonsterDeathAnimationCompletedEvent;
	struct MonsterSpawnResult;
	struct QamilDefeatedEvent;
	class PlayerControlComponent;
	class QamilHealthWidget;
	class QamilStateMachineComponent;

	class QamilScene : public GameplayScene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnExit() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;

	private:
		enum class EncounterPhase
		{
			WaitingForPlayback,
			PreBattleCombat,
			BossPreview,
			BossCombatScheduled,
			BossCombat,
			BossDefeated,
		};

		enum class DefeatPresentationPhase
		{
			Inactive,
			SlowMotion,
			Covered,
		};

		bool InitializePreBattleMonsters(const std::vector<MonsterSpawnResult>& monsterSpawnResults);
		void UpdateEncounter();
		float CalculatePreviewFightBeat() const;
		void StartPreBattleCombat();
		bool TrackPreBattleMonsters();
		void DisconnectPreBattleMonsterEvents();
		void HandlePreBattleMonsterDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent& event);
		void BeginBossPreview();
		void ScheduleBossCombat();
		void StartBossCombat();
		void HandleQamilDefeated(const QamilDefeatedEvent& event);
		void UpdateDefeatPresentation();
		void RestoreDefeatTimeScale();
		bool PlacePlayerForBossPreview();
		void SetQamilCollidersEnabled(bool enabled);

		WeakGameObjectPtr								_qamil{};
		std::vector<WeakGameObjectPtr>					_preBattleMonsters{};
		std::vector<std::unique_ptr<EventConnection>>	_preBattleMonsterDeathConnections{};
		EventConnection								_qamilDefeatedConnection{};
		PlayerControlComponent*							_playerControlComponent = nullptr;
		QamilHealthWidget*								_qamilHealthWidget = nullptr;
		QamilStateMachineComponent*						_qamilStateMachine = nullptr;
		std::optional<float>							_fightBeat{};
		std::optional<float>							_combatStartBeat{};
		uint32											_remainingPreBattleMonsterCount = 0;
		EncounterPhase									_encounterPhase = EncounterPhase::WaitingForPlayback;
		DefeatPresentationPhase							_defeatPresentationPhase = DefeatPresentationPhase::Inactive;
		float											_defeatPresentationElapsed = 0.f;
		float											_timeScaleBeforeDefeat = 1.f;
		bool											_isDefeatTimeScaleActive = false;
	};
}
