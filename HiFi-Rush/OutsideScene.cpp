#include "OutsideScene.h"
#include "Application.h"
#include "AudioStatics.h"
#include "BeatSystem.h"
#include "CameraManager.h"
#include "DialogComponent.h"
#include "GameObject.h"
#include "GameplayAnnouncementWidget.h"
#include "HiFiRushAudio.h"
#include "HiFiRushDialogData.h"
#include "HiFiRushStatics.h"
#include "Input.h"
#include "MathUtil.h"
#include "MonsterStateMachineComponent.h"
#include "MonsterSpawner.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSpawner.h"
#include "Resources.h"
#include "SceneDebugTools.h"
#include "TriggerSystem.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t FirstWaveTriggerId[] = L"Outside.FirstWave";
		constexpr wchar_t SecondWaveTriggerId[] = L"Outside.SecondWave";
		constexpr wchar_t FightDoorTriggerId[] = L"Outside.FirstFightDoor";
		constexpr float TriggerAnimationBeatOffset = 2.f;
	}

	void OutsideScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);

		const std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"jump_outside");
		GM_ASSERT_RETURN(navigationMesh, "jump_outside NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);

		APPLICATION.GetInput().SetCursorLocked(true);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Outside);
		InitializeGameplayUI();
		SetGameplayStatusUIVisible(false);
		GetDialogComponent().OnFinished.Subscribe(_dialogFinishedConnection,
			[this](const DialogFinishedEvent& event)
			{
				HandleDialogFinished(event);
			});
		GM_ASSERT_RETURN(PlayDialogSequence(HiFiRushDialogSequenceIds::SaverEncounter), "Outside Encounter Dialog 재생에 실패했습니다.");
	}

	void OutsideScene::OnExit()
	{
		_dialogFinishedConnection.Disconnect();
		DisconnectWaveDeathAnimationEvents();
		_firstWaveMonsters.clear();
		_secondWaveMonsters.clear();
		_triggerSoundBeat.reset();
		_encounterPhase = EncounterPhase::Intro;
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void OutsideScene::OnInitialize()
	{
		std::vector<MonsterSpawnResult> monsterSpawnResults;
		GM_ASSERT_RETURN(InitializeMap(L"OutsideMap", monsterSpawnResults), "Outside Map 구성에 실패했습니다.");
		GM_ASSERT_RETURN(InitializeEncounterMonsters(monsterSpawnResults), "Outside Encounter Monster 구성에 실패했습니다.");

		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ -0.1f, 0.f, 11.f };
		playerDesc.rotationY = Math::GM_PI;
		playerDesc.cameraDistance = 4.f;
		playerDesc.cameraYaw = Math::DegreesToRadians(180.f);
		GM_ASSERT_RETURN(InitializePlayer(playerDesc), "Outside Player 생성에 실패했습니다.");
	}

	void OutsideScene::OnTick(float deltaTime)
	{
		UpdateEncounter();
		TickSceneTransitionDebug();
	}

	bool OutsideScene::InitializeEncounterMonsters(const std::vector<MonsterSpawnResult>& monsterSpawnResults)
	{
		for (const MonsterSpawnResult& spawnResult : monsterSpawnResults)
		{
			if (spawnResult.activationTriggerId == FirstWaveTriggerId)
				_firstWaveMonsters.push_back(spawnResult.monster);
			else if (spawnResult.activationTriggerId == SecondWaveTriggerId)
				_secondWaveMonsters.push_back(spawnResult.monster);
		}

		return true;
	}

	void OutsideScene::HandleDialogFinished(const DialogFinishedEvent& event)
	{
		if (event.completed == false || event.sequenceId != HiFiRushDialogSequenceIds::SaverEncounter)
			return;

		StartEncounter();
	}

	void OutsideScene::StartEncounter()
	{
		if (_encounterPhase != EncounterPhase::Intro)
			return;

		SetGameplayStatusUIVisible(true);
		PlayAnnouncement(GameplayAnnouncementType::Fight);
		GM_ASSERT_RETURN(GetTriggerSystem().Activate(FirstWaveTriggerId), "Outside 첫 번째 Wave Trigger 활성화에 실패했습니다.");
		GM_ASSERT_RETURN(TrackWaveMonsters(_firstWaveMonsters), "Outside 첫 번째 Wave 추적에 실패했습니다.");
		_encounterPhase = EncounterPhase::FirstWave;
	}

	bool OutsideScene::TrackWaveMonsters(const std::vector<WeakGameObjectPtr>& monsters)
	{
		DisconnectWaveDeathAnimationEvents();
		_remainingWaveMonsterCount = static_cast<uint32>(monsters.size());
		_waveDeathAnimationConnections.reserve(monsters.size());

		for (const WeakGameObjectPtr& monsterReference : monsters)
		{
			GameObject* monster = monsterReference.Get();
			GM_ASSERT_RETURN_VAL(monster, false, "Outside Wave Monster가 유효하지 않습니다.");
			MonsterStateMachineComponent* stateMachine = monster->GetComponent<MonsterStateMachineComponent>();
			GM_ASSERT_RETURN_VAL(stateMachine, false, "Outside Wave Monster에 MonsterStateMachineComponent가 없습니다.");

			auto deathAnimationConnection = std::make_unique<EventConnection>();
			stateMachine->OnDeathAnimationCompleted.Subscribe(*deathAnimationConnection,
				[this](const MonsterDeathAnimationCompletedEvent& event)
				{
					HandleMonsterDeathAnimationCompleted(event);
				});
			_waveDeathAnimationConnections.push_back(std::move(deathAnimationConnection));
		}

		return true;
	}

	void OutsideScene::DisconnectWaveDeathAnimationEvents()
	{
		_waveDeathAnimationConnections.clear();
		_remainingWaveMonsterCount = 0;
	}

	void OutsideScene::HandleMonsterDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent&)
	{
		if (_remainingWaveMonsterCount == 0)
			return;

		--_remainingWaveMonsterCount;
		if (_remainingWaveMonsterCount > 0)
			return;

		DisconnectWaveDeathAnimationEvents();
		if (_encounterPhase == EncounterPhase::FirstWave)
			ScheduleSecondWave();
		else if (_encounterPhase == EncounterPhase::SecondWave)
			CompleteEncounter();
	}

	void OutsideScene::ScheduleSecondWave()
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		GM_ASSERT_RETURN(beatSystem.HasPlaybackTime(), "Outside 두 번째 Wave를 예약할 Beat 재생 시간이 없습니다.");
		const float triggerStartBeat = beatSystem.GetNextBeat();
		GM_ASSERT_RETURN(GetTriggerSystem().Activate(SecondWaveTriggerId), "Outside 두 번째 Wave Trigger 활성화에 실패했습니다.");
		_triggerSoundBeat = triggerStartBeat + TriggerAnimationBeatOffset;
		GM_ASSERT_RETURN(TrackWaveMonsters(_secondWaveMonsters), "Outside 두 번째 Wave 추적에 실패했습니다.");
		_encounterPhase = EncounterPhase::SecondWave;
	}

	void OutsideScene::CompleteEncounter()
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		GM_ASSERT_RETURN(beatSystem.HasPlaybackTime(), "Outside 전투 종료 연출을 예약할 Beat 재생 시간이 없습니다.");
		const float triggerStartBeat = beatSystem.GetNextBeat();
		GM_ASSERT_RETURN(GetTriggerSystem().Activate(FightDoorTriggerId), "Outside 전투 종료 Door Trigger 활성화에 실패했습니다.");
		_triggerSoundBeat = triggerStartBeat;
		_encounterPhase = EncounterPhase::Completed;
	}

	void OutsideScene::UpdateEncounter()
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false)
			return;

		const float currentBeat = beatSystem.GetCurrentBeat();
		if (_triggerSoundBeat && currentBeat >= *_triggerSoundBeat)
		{
			PlaySound2D(HiFiRushSound::OutsideTriggerOpen, 0.5f);
			_triggerSoundBeat.reset();
		}

	}

}
