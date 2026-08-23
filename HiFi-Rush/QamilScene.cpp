#include "QamilScene.h"
#include "Application.h"
#include "BeatSystem.h"
#include "CameraComponent.h"
#include "CameraFollowComponent.h"
#include "CameraManager.h"
#include "ChiStateMachineComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayAnnouncementWidget.h"
#include "GMLog.h"
#include "HealthComponent.h"
#include "HiFiRushAudio.h"
#include "HiFiRushStatics.h"
#include "Input.h"
#include "MathUtil.h"
#include "MonsterSpawner.h"
#include "MonsterStateMachineComponent.h"
#include "NavigationMesh.h"
#include "NavMeshControllerComponent.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "PlayerControlComponent.h"
#include "PlayerSpawner.h"
#include "QamilHealthWidget.h"
#include "QamilPhaseTriggerComponent.h"
#include "QamilSpawner.h"
#include "QamilStateMachineComponent.h"
#include "Resources.h"
#include "Rigidbody3DComponent.h"
#include "SceneDebugTools.h"
#include "TransformComponent.h"
#include "TriggerSystem.h"
#include "TimeSystem.h"
#include "UIManager.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t PreBattleTriggerId[] = L"Qamil.PreBattle";
		constexpr uint32 PreBattleMonsterCount = 4;
		constexpr float BossPreviewDurationSeconds = 3.f;
		constexpr Vector3 PreBattlePlayerPosition{ -0.43f, 0.f, -13.5f };
		constexpr float PreBattlePlayerRotationY = 0.f;
		constexpr float PreBattleCameraDistance = 7.f;
		constexpr float PreBattleCameraYaw = 0.f;
		constexpr float PreBattleCameraPitch = 0.f;
		constexpr float PreBattleCameraHeight = 0.f;
		constexpr Vector3 BossPreviewPlayerPosition{ -0.65f, -0.02f, -4.93f };
		constexpr float BossPreviewPlayerRotationY = 0.f;
		constexpr float BossPreviewCameraDistance = 7.f;
		constexpr float BossPreviewCameraYaw = Math::DegreesToRadians(19.6f);
		constexpr float BossPreviewCameraPitch = Math::DegreesToRadians(-53.7f);
		constexpr float BossPreviewCameraHeight = 0.f;
		constexpr float QamilDefeatSlowMotionDuration = 2.f;
		constexpr float QamilDefeatTimeScale = 0.05f;
	}

	void QamilScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);

		const std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"qamil");
		GM_ASSERT_RETURN(navigationMesh, "qamil NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);

		APPLICATION.GetInput().SetCursorLocked(true);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Qamil);
		InitializeGameplayUI();
		SetBossBattleHUDEnabled(false);
		GM_ASSERT_RETURN(_playerControlComponent, "Qamil 전투에 PlayerControlComponent가 필요합니다.");
		GetTriggerSystem().Reset(PreBattleTriggerId);
		GetTriggerSystem().Reset(QamilPhase2TriggerId);
		GetTriggerSystem().Reset(QamilPhase3TriggerId);
		_qamil->SetRender(false);
		SetQamilCollidersEnabled(false);
		_fightBeat.reset();
		_combatStartBeat.reset();
		_encounterPhase = EncounterPhase::WaitingForPlayback;

		GameObject* qamil = _qamil.Get();
		GM_ASSERT_RETURN(qamil, "Qamil Health UI를 구성하려면 Qamil이 필요합니다.");
		_qamilStateMachine = qamil->GetComponent<QamilStateMachineComponent>();
		GM_ASSERT_RETURN(_qamilStateMachine, "Qamil 전투 운영에 QamilStateMachineComponent가 필요합니다.");
		GM_ASSERT_RETURN(_qamilStateMachine->StopCombat(), "Qamil 비전투 상태 초기화에 실패했습니다.");
		HealthComponent* healthComponent = qamil->GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(healthComponent, "Qamil Health UI를 구성하려면 Qamil HealthComponent가 필요합니다.");
		_qamilHealthWidget = APPLICATION.GetUIManager().AddUserWidget<QamilHealthWidget>(HiFiRushStatics::GetBeatSystem(), *healthComponent);
		GM_ASSERT_RETURN(_qamilHealthWidget, "Qamil Health UI 생성에 실패했습니다.");
		_qamilHealthWidget->SetVisible(false);
		_qamilStateMachine->OnDefeated.Subscribe(_qamilDefeatedConnection, [this](const QamilDefeatedEvent& event) { HandleQamilDefeated(event); });
		_defeatPresentationPhase = DefeatPresentationPhase::Inactive;
		_defeatPresentationElapsed = 0.f;
	}

	void QamilScene::OnExit()
	{
		RestoreDefeatTimeScale();
		DisconnectPreBattleMonsterEvents();
		_qamilDefeatedConnection.Disconnect();
		SetBossBattleHUDEnabled(false);
		if (_playerControlComponent)
			_playerControlComponent->ReleaseControls(this);
		_preBattleMonsters.clear();
		_playerControlComponent = nullptr;
		_qamilHealthWidget = nullptr;
		_qamilStateMachine = nullptr;
		_fightBeat.reset();
		_combatStartBeat.reset();
		_encounterPhase = EncounterPhase::WaitingForPlayback;
		_defeatPresentationPhase = DefeatPresentationPhase::Inactive;
		_defeatPresentationElapsed = 0.f;
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void QamilScene::OnInitialize()
	{
		std::vector<MonsterSpawnResult> monsterSpawnResults;
		GM_ASSERT_RETURN(InitializeMap(L"QamilMap", monsterSpawnResults), "Qamil Map 구성에 실패했습니다.");
		GM_ASSERT_RETURN(InitializePreBattleMonsters(monsterSpawnResults), "Qamil 선행 전투 Monster 구성에 실패했습니다.");

		QamilSpawner qamilSpawner(APPLICATION.GetResources());
		QamilSpawnDesc qamilDesc{};
		GameObject* qamil = qamilSpawner.Spawn(*this, qamilDesc);
		GM_ASSERT_RETURN(qamil, "Qamil 생성에 실패했습니다.");
		_qamil = qamil->GetWeakPtr();

		PlayerSpawnDesc playerDesc{};
		playerDesc.position = PreBattlePlayerPosition;
		playerDesc.rotationY = PreBattlePlayerRotationY;
		playerDesc.cameraDistance = PreBattleCameraDistance;
		playerDesc.cameraYaw = PreBattleCameraYaw;
		playerDesc.cameraPitch = PreBattleCameraPitch;
		playerDesc.cameraHeight = PreBattleCameraHeight;
		GM_ASSERT_RETURN(InitializePlayer(playerDesc), "Player 생성에 실패했습니다.");
		GameObject* player = GetPlayer().Get();
		_playerControlComponent = player->GetComponent<PlayerControlComponent>();
	}

	void QamilScene::OnTick(float)
	{
	#if GM_ENABLE_DEBUG_TOOLS
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::F8))
		{
			GameObject* qamil = _qamil.Get();
			HealthComponent* healthComponent = qamil ? qamil->GetComponent<HealthComponent>() : nullptr;
			if (healthComponent)
			{
				const int32 halfHealth = healthComponent->GetMaxHealth() / 2;
				healthComponent->SetHealth((std::min)(healthComponent->GetHealth(), halfHealth));
				GM_LOG("[Qamil Debug] Health: %d / %d", healthComponent->GetHealth(), healthComponent->GetMaxHealth());
			}
		}
	#endif
		UpdateDefeatPresentation();
		UpdateEncounter();
		TickSceneTransitionDebug();
	}

	bool QamilScene::InitializePreBattleMonsters(const std::vector<MonsterSpawnResult>& monsterSpawnResults)
	{
		_preBattleMonsters.clear();
		for (const MonsterSpawnResult& spawnResult : monsterSpawnResults)
		{
			if (spawnResult.activationTriggerId == PreBattleTriggerId)
				_preBattleMonsters.push_back(spawnResult.monster);
		}

		GM_ASSERT_RETURN_VAL(_preBattleMonsters.size() == PreBattleMonsterCount, false, "Qamil 선행 전투 Monster 수가 유효하지 않습니다. count=%zu", _preBattleMonsters.size());
		return true;
	}

	void QamilScene::UpdateEncounter()
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false)
			return;

		const float currentBeat = beatSystem.GetCurrentBeat();
		switch (_encounterPhase)
		{
		case EncounterPhase::WaitingForPlayback:
			StartPreBattleCombat();
			break;
		case EncounterPhase::BossPreview:
			if (_fightBeat && currentBeat >= *_fightBeat - 1.f)
				ScheduleBossCombat();
			break;
		case EncounterPhase::BossCombatScheduled:
			if (_combatStartBeat && currentBeat >= *_combatStartBeat)
				StartBossCombat();
			break;
		default:
			break;
		}
	}

	float QamilScene::CalculatePreviewFightBeat() const
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		GM_ASSERT_RETURN_VAL(beatSystem.HasPlaybackTime(), 0.f, "Qamil 전투 Preview 시간을 계산할 Beat 재생 시간이 없습니다.");
		const float secondsPerBeat = beatSystem.GetSecondsPerBeat();
		GM_ASSERT_RETURN_VAL(secondsPerBeat > 0.f, 0.f, "Qamil 전투 Preview 시간을 계산할 Seconds Per Beat가 유효하지 않습니다.");
		return std::ceil(beatSystem.GetCurrentBeat() + BossPreviewDurationSeconds / secondsPerBeat);
	}

	void QamilScene::StartPreBattleCombat()
	{
		GM_ASSERT_RETURN(_playerControlComponent, "Qamil 선행 전투를 시작할 PlayerControlComponent가 없습니다.");
		GM_ASSERT_RETURN(TrackPreBattleMonsters(), "Qamil 선행 전투 Monster 추적에 실패했습니다.");
		GM_ASSERT_RETURN(GetTriggerSystem().Activate(PreBattleTriggerId), "Qamil 선행 전투 Trigger 활성화에 실패했습니다.");
		PlayAnnouncement(GameplayAnnouncementType::Fight);
		_playerControlComponent->ReleaseControls(this);
		_encounterPhase = EncounterPhase::PreBattleCombat;
	}

	bool QamilScene::TrackPreBattleMonsters()
	{
		DisconnectPreBattleMonsterEvents();
		_remainingPreBattleMonsterCount = static_cast<uint32>(_preBattleMonsters.size());
		_preBattleMonsterDeathConnections.reserve(_preBattleMonsters.size());
		for (const WeakGameObjectPtr& monsterReference : _preBattleMonsters)
		{
			GameObject* monster = monsterReference.Get();
			GM_ASSERT_RETURN_VAL(monster, false, "Qamil 선행 전투 Monster가 유효하지 않습니다.");
			MonsterStateMachineComponent* stateMachine = monster->GetComponent<MonsterStateMachineComponent>();
			GM_ASSERT_RETURN_VAL(stateMachine, false, "Qamil 선행 전투 Monster에 MonsterStateMachineComponent가 없습니다.");
			auto deathAnimationConnection = std::make_unique<EventConnection>();
			stateMachine->OnDeathAnimationCompleted.Subscribe(*deathAnimationConnection, [this](const MonsterDeathAnimationCompletedEvent& event) { HandlePreBattleMonsterDeathAnimationCompleted(event); });
			_preBattleMonsterDeathConnections.push_back(std::move(deathAnimationConnection));
		}

		return true;
	}

	void QamilScene::DisconnectPreBattleMonsterEvents()
	{
		_preBattleMonsterDeathConnections.clear();
		_remainingPreBattleMonsterCount = 0;
	}

	void QamilScene::HandlePreBattleMonsterDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent&)
	{
		if (_encounterPhase != EncounterPhase::PreBattleCombat || _remainingPreBattleMonsterCount == 0)
			return;

		--_remainingPreBattleMonsterCount;
		if (_remainingPreBattleMonsterCount > 0)
			return;

		DisconnectPreBattleMonsterEvents();
		BeginBossPreview();
	}

	void QamilScene::BeginBossPreview()
	{
		PlayScreenWipe();
		_playerControlComponent->BlockControls(this, PlayerControl::All);
		GM_ASSERT_RETURN(PlacePlayerForBossPreview(), "Qamil Boss 연출을 위한 Player 배치에 실패했습니다.");
		_qamil->SetRender(true);
		SetQamilCollidersEnabled(false);
		if (_qamilHealthWidget)
			_qamilHealthWidget->SetVisible(false);
		_fightBeat = CalculatePreviewFightBeat();
		_combatStartBeat.reset();
		_encounterPhase = EncounterPhase::BossPreview;
	}

	void QamilScene::ScheduleBossCombat()
	{
		_combatStartBeat = HiFiRushStatics::GetBeatSystem().GetNextBeat();
		_encounterPhase = EncounterPhase::BossCombatScheduled;
	}

	void QamilScene::StartBossCombat()
	{
		GM_ASSERT_RETURN(_qamilStateMachine && _qamilStateMachine->StartCombat(), "Qamil Boss 상태 머신 시작에 실패했습니다.");
		SetQamilCollidersEnabled(true);
		if (_qamilHealthWidget)
			_qamilHealthWidget->SetVisible(true);
		SetBossBattleHUDEnabled(true);
		PlayAnnouncement(GameplayAnnouncementType::Fight);
		_playerControlComponent->ReleaseControls(this);
		_encounterPhase = EncounterPhase::BossCombat;
	}

	void QamilScene::HandleQamilDefeated(const QamilDefeatedEvent&)
	{
		if (_encounterPhase != EncounterPhase::BossCombat || _defeatPresentationPhase != DefeatPresentationPhase::Inactive)
			return;

		SetBossBattleHUDEnabled(false);
		if (_qamilHealthWidget)
			_qamilHealthWidget->SetVisible(false);

		CameraShakeDesc shakeDesc{};
		shakeDesc.duration = QamilDefeatSlowMotionDuration;
		shakeDesc.locationAmplitude = Vector3{ 0.5f, 0.36f, 0.16f };
		shakeDesc.rotationAmplitude = Vector3{ 0.06f, 0.048f, 0.08f };
		GetCameraManager()->AddShake(shakeDesc);

		TimeSystem& timeSystem = APPLICATION.GetTimeSystem();
		_timeScaleBeforeDefeat = timeSystem.GetTimeScale();
		timeSystem.SetTimeScale(QamilDefeatTimeScale);
		_isDefeatTimeScaleActive = true;
		_defeatPresentationElapsed = 0.f;
		_defeatPresentationPhase = DefeatPresentationPhase::SlowMotion;
		_encounterPhase = EncounterPhase::BossDefeated;
	}

	void QamilScene::UpdateDefeatPresentation()
	{
		if (_defeatPresentationPhase == DefeatPresentationPhase::Inactive || _defeatPresentationPhase == DefeatPresentationPhase::Covered)
			return;

		_defeatPresentationElapsed += APPLICATION.GetTimeSystem().GetUnscaledDeltaTime();
		if (_defeatPresentationElapsed < QamilDefeatSlowMotionDuration)
			return;

		RestoreDefeatTimeScale();
		CoverScreenWithWipe();
		_defeatPresentationPhase = DefeatPresentationPhase::Covered;
	}

	void QamilScene::RestoreDefeatTimeScale()
	{
		if (_isDefeatTimeScaleActive == false)
			return;

		APPLICATION.GetTimeSystem().SetTimeScale(_timeScaleBeforeDefeat);
		_isDefeatTimeScaleActive = false;
	}

	bool QamilScene::PlacePlayerForBossPreview()
	{
		GameObject* player = GetPlayer().Get();
		GameObject* qamil = _qamil.Get();
		CameraComponent* camera = GetCameraManager()->GetActiveCamera();
		GM_ASSERT_RETURN_VAL(player && qamil && camera, false, "Qamil Boss 연출에 필요한 Player, Qamil 또는 Camera가 유효하지 않습니다.");
		TransformComponent* playerTransform = player->GetTransform();
		TransformComponent* qamilTransform = qamil->GetTransform();
		Rigidbody3DComponent* rigidbody = player->GetRigidbody3D();
		NavMeshControllerComponent* navMeshController = player->GetComponent<NavMeshControllerComponent>();
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		CameraFollowComponent* cameraFollow = camera->GetOwner().GetComponent<CameraFollowComponent>();
		GM_ASSERT_RETURN_VAL(playerTransform && qamilTransform && rigidbody && navMeshController && stateMachine && cameraFollow, false, "Qamil Boss 연출에 필요한 Player Component가 없습니다.");
		playerTransform->SetPosition(BossPreviewPlayerPosition);
		playerTransform->SetRotationY(BossPreviewPlayerRotationY);
		qamilTransform->SetRotationY(Math::GM_PI);
		rigidbody->SetVelocity(Vector3{});
		rigidbody->ClearForces();
		navMeshController->SetCurrentCellIndex(-1);
		GM_ASSERT_RETURN_VAL(navMeshController->RefreshCellIndex(), false, "Qamil Boss 연출 Player 위치의 Navigation Cell을 찾지 못했습니다.");
		stateMachine->ChangeState(ChiStateId::Idle);
		cameraFollow->SetDistance(BossPreviewCameraDistance);
		cameraFollow->SetYaw(BossPreviewCameraYaw);
		cameraFollow->SetPitch(BossPreviewCameraPitch);
		cameraFollow->SetHeight(BossPreviewCameraHeight);
		SetPlayerRespawnPoint(BossPreviewPlayerPosition, BossPreviewPlayerRotationY);
		return true;
	}

	void QamilScene::SetQamilCollidersEnabled(bool enabled)
	{
		GameObject* qamil = _qamil.Get();
		GM_ASSERT_RETURN(qamil, "Collider 상태를 변경할 Qamil이 유효하지 않습니다.");
		for (Collider3DComponent* collider : qamil->GetColliders3D())
			collider->SetEnabled(enabled);
	}

}
