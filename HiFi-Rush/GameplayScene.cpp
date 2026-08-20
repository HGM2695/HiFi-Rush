#include "GameplayScene.h"
#include "Application.h"
#include "AudioComponent.h"
#include "BeatHitWidget.h"
#include "ChiStateMachineComponent.h"
#include "Collider3DComponent.h"
#include "ComboResultWidget.h"
#include "DialogComponent.h"
#include "DialogWidget.h"
#include "GameplayAnnouncementWidget.h"
#include "RhythmBarWidget.h"
#include "RhythmMeterWidget.h"
#include "RhythmRankComponent.h"
#include "EnvironmentSpawner.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "HealthComponent.h"
#include "HiFiRushStatics.h"
#include "HiFiRushDialogData.h"
#include "MapResource.h"
#include "MonsterSpawner.h"
#include "PlayerControlComponent.h"
#include "PlayerControlTypes.h"
#include "PlayerSpawner.h"
#include "PlayerStatusWidget.h"
#include "Resources.h"
#include "ReverbComponent.h"
#include "ScreenWipeWidget.h"
#include "Rigidbody3DComponent.h"
#include "SoundWave.h"
#include "TransformComponent.h"
#include "TriggerSystem.h"
#include "UIManager.h"
#include "NavMeshControllerComponent.h"

#include <cmath>
#include <memory>

namespace gm
{
	GameplayScene::GameplayScene()
		: _triggerSystem(std::make_unique<TriggerSystem>(HiFiRushStatics::GetBeatSystem()))
	{}

	GameplayScene::~GameplayScene() = default;

	TriggerSystem& GameplayScene::GetTriggerSystem()
	{
		return *_triggerSystem;
	}

	const TriggerSystem& GameplayScene::GetTriggerSystem() const
	{
		return *_triggerSystem;
	}

	DialogComponent& GameplayScene::GetDialogComponent()
	{
		GM_ASSERT_TERMINATE(_dialogComponent, "DialogComponent가 구성되지 않았습니다.");
		return *_dialogComponent;
	}

	const DialogComponent& GameplayScene::GetDialogComponent() const
	{
		GM_ASSERT_TERMINATE(_dialogComponent, "DialogComponent가 구성되지 않았습니다.");
		return *_dialogComponent;
	}

	bool GameplayScene::PlayDialogSequence(const std::wstring& sequenceId)
	{
		GM_ASSERT_RETURN_VAL(_dialogComponent, false, "DialogComponent가 구성되지 않았습니다.");
		return _dialogComponent->PlaySequence(sequenceId);
	}

	bool GameplayScene::SelectDialogBranch(const std::wstring& branchKey)
	{
		GM_ASSERT_RETURN_VAL(_dialogComponent, false, "DialogComponent가 구성되지 않았습니다.");
		return _dialogComponent->SelectBranch(branchKey);
	}

	void GameplayScene::PlayAnnouncement(GameplayAnnouncementType type)
	{
		GM_ASSERT_RETURN(_announcementWidget, "Gameplay Announcement Widget이 구성되지 않았습니다.");
		_announcementWidget->Play(type);
	}

	void GameplayScene::SetGameplayStatusUIVisible(bool isVisible)
	{
		GM_ASSERT_RETURN(_playerStatusWidget && _rhythmMeterWidget, "Gameplay Status UI가 구성되지 않았습니다.");
		_playerStatusWidget->SetVisible(isVisible);
		_rhythmMeterWidget->SetVisible(isVisible);
	}

	void GameplayScene::SetBossBattleHUDEnabled(bool enabled)
	{
		GM_ASSERT_RETURN(_rhythmBarWidget, "Boss Battle HUD를 구성하려면 RhythmBarWidget이 필요합니다.");
		_rhythmBarWidget->SetBossLayoutEnabled(enabled);
	}

	void GameplayScene::SetPlayerRespawnPoint(const Vector3& position, float rotationY)
	{
		_playerRespawnPosition = position;
		_playerRespawnRotationY = rotationY;
	}

	void GameplayScene::HandlePlayerFall(int32 damage)
	{
		GM_ASSERT_RETURN(damage > 0, "낙사 Damage는 0보다 커야 합니다.");
		if (_isPlayerDead)
			return;

		GameObject* player = _player.Get();
		GM_ASSERT_RETURN(player, "낙사 처리할 Player가 유효하지 않습니다.");
		HealthComponent* healthComponent = player->GetComponent<HealthComponent>();
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(healthComponent && stateMachine, "Player 낙사 처리에 필요한 Component가 없습니다.");
		GM_ASSERT_RETURN(ReturnPlayerToRespawnPoint(), "Player를 Respawn Point로 복귀시키지 못했습니다.");

		DamageInfo damageInfo{};
		damageInfo.amount = damage;
		damageInfo.ignoreInvincibility = true;
		const DamageResult damageResult = healthComponent->ApplyDamage(damageInfo);
		if (damageResult.state == DamageState::Applied && damageResult.isDead == false)
			stateMachine->ChangeState(ChiStateId::Idle);
	}

	bool GameplayScene::InitializeMap(const std::wstring& mapResourceKey)
	{
		std::vector<MonsterSpawnResult> unusedMonsterSpawnResults;
		return InitializeMap(mapResourceKey, unusedMonsterSpawnResults);
	}

	bool GameplayScene::InitializeMap(const std::wstring& mapResourceKey, std::vector<MonsterSpawnResult>& outMonsterSpawnResults)
	{
		const std::shared_ptr<MapResource> mapResource = APPLICATION.GetResources().Find<MapResource>(mapResourceKey);
		GM_ASSERT_RETURN_VAL(mapResource, false, "MapResource가 로드되지 않았습니다. key=%ls", mapResourceKey.c_str());

		const MapData& mapData = mapResource->GetData();

		EnvironmentSpawner environmentSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(environmentSpawner.Spawn(*this, mapData.objects), false, "환경 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

		MonsterSpawner monsterSpawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN_VAL(monsterSpawner.SpawnAll(*this, mapData.monsterSpawnDatas, outMonsterSpawnResults), false, "Monster 구성에 실패했습니다. key=%ls", mapResourceKey.c_str());

		return true;
	}

	bool GameplayScene::InitializePlayer(const PlayerSpawnDesc& desc)
	{
		PlayerSpawner playerSpawner(APPLICATION.GetResources());
		GameObject* player = playerSpawner.Spawn(*this, desc, HiFiRushStatics::GetPlayerRuntimeState());
		if (player == nullptr)
			return false;

		_player = player->GetWeakPtr();
		SetPlayerRespawnPoint(desc.position, desc.rotationY);
		return true;
	}

	void GameplayScene::InitializeGameplayUI()
	{
		GameObject* player = _player.Get();
		GM_ASSERT_RETURN(player, "Gameplay UI를 구성하려면 Player가 필요합니다.");

		HealthComponent* healthComponent = player->GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(healthComponent, "PlayerStatusWidget을 구성하려면 Player HealthComponent가 필요합니다.");
		ReverbComponent* reverbComponent = player->GetComponent<ReverbComponent>();
		GM_ASSERT_RETURN(reverbComponent, "PlayerStatusWidget을 구성하려면 Player ReverbComponent가 필요합니다.");
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(stateMachine, "PlayerStatusWidget을 구성하려면 ChiStateMachineComponent가 필요합니다.");
		RhythmRankComponent* rhythmRankComponent = player->GetComponent<RhythmRankComponent>();
		GM_ASSERT_RETURN(rhythmRankComponent, "RhythmMeterWidget을 구성하려면 RhythmRankComponent가 필요합니다.");
		PlayerControlComponent* playerControlComponent = player->GetComponent<PlayerControlComponent>();
		GM_ASSERT_RETURN(playerControlComponent, "Dialog을 구성하려면 PlayerControlComponent가 필요합니다.");

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();

		GameObject* dialogObject = SpawnGameObject<GameObject>();
		AudioComponent* dialogAudio = dialogObject->AddComponent<AudioComponent>(std::shared_ptr<SoundWave>{});
		GM_ASSERT_RETURN(dialogAudio, "Dialog AudioComponent 생성에 실패했습니다.");
		_dialogComponent = dialogObject->AddComponent<DialogComponent>(*dialogAudio, *playerControlComponent);
		GM_ASSERT_RETURN(_dialogComponent, "DialogComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN(_dialogComponent->SetSequences(CreateHiFiRushDialogSequences()), "Hi-Fi RUSH Dialog Sequence 구성에 실패했습니다.");

		_playerStatusWidget = uiManager.AddUserWidget<PlayerStatusWidget>(HiFiRushStatics::GetBeatSystem(), *healthComponent, *reverbComponent, *stateMachine);
		_rhythmMeterWidget = uiManager.AddUserWidget<RhythmMeterWidget>(HiFiRushStatics::GetBeatSystem(), *rhythmRankComponent);
		_rhythmBarWidget = uiManager.AddUserWidget<RhythmBarWidget>(HiFiRushStatics::GetBeatSystem());
		uiManager.AddUserWidget<ComboResultWidget>(*stateMachine);
		uiManager.AddUserWidget<BeatHitWidget>(HiFiRushStatics::GetBeatSystem(), *stateMachine);
		_announcementWidget = uiManager.AddUserWidget<GameplayAnnouncementWidget>(HiFiRushStatics::GetBeatSystem());
		uiManager.AddUserWidget<DialogWidget>(HiFiRushStatics::GetBeatSystem(), *_dialogComponent);
		_screenWipeWidget = uiManager.AddUserWidget<ScreenWipeWidget>();

		_playerDeathConnection.Disconnect();
		_playerDeathAnimationCompletedConnection.Disconnect();
		healthComponent->OnDeath.Subscribe(_playerDeathConnection,
			[this](const HitEvent& event)
			{
				HandlePlayerDeath(event);
			});

		stateMachine->OnDeathAnimationCompleted.Subscribe(_playerDeathAnimationCompletedConnection,
			[this](const PlayerDeathAnimationCompletedEvent& event)
			{
				HandlePlayerDeathAnimationCompleted(event);
			});
	}

	void GameplayScene::PlayScreenWipe()
	{
		GM_ASSERT_RETURN(_screenWipeWidget, "Screen Wipe Widget이 구성되지 않았습니다.");
		_screenWipeWidget->PlayOpen();
	}

	void GameplayScene::CoverScreenWithWipe()
	{
		GM_ASSERT_RETURN(_screenWipeWidget, "Screen Wipe Widget이 구성되지 않았습니다.");
		_screenWipeWidget->PlayCover();
	}

	void GameplayScene::HandlePlayerDeath(const HitEvent&)
	{
		if (_isPlayerDead)
			return;

		GameObject* player = _player.Get();
		GM_ASSERT_RETURN(player, "사망 처리할 Player가 유효하지 않습니다.");
		PlayerControlComponent* controlComponent = player->GetComponent<PlayerControlComponent>();
		GM_ASSERT_RETURN(controlComponent, "Player 사망 처리에는 PlayerControlComponent가 필요합니다.");

		_isPlayerDead = true;
		controlComponent->BlockControls(this, PlayerControl::Movement | PlayerControl::Action);

		_disabledPlayerColliders.clear();
		for (Collider3DComponent* collider : player->GetColliders3D())
		{
			if (collider == nullptr || collider->IsEnabled() == false)
				continue;

			collider->SetEnabled(false);
			_disabledPlayerColliders.push_back(collider);
		}
	}

	void GameplayScene::HandlePlayerDeathAnimationCompleted(const PlayerDeathAnimationCompletedEvent&)
	{
		if (_isPlayerDead)
			RespawnPlayer();
	}

	bool GameplayScene::ReturnPlayerToRespawnPoint()
	{
		GameObject* player = _player.Get();
		GM_ASSERT_RETURN_VAL(player, false, "복귀시킬 Player가 유효하지 않습니다.");
		TransformComponent* transform = player->GetTransform();
		Rigidbody3DComponent* rigidbody = player->GetRigidbody3D();
		NavMeshControllerComponent* navMeshController = player->GetComponent<NavMeshControllerComponent>();
		GM_ASSERT_RETURN_VAL(transform && rigidbody && navMeshController, false, "Player 복귀에 필요한 Component가 없습니다.");
		GM_ASSERT_RETURN_VAL(_screenWipeWidget, false, "Screen Wipe Widget이 구성되지 않았습니다.");
		PlayScreenWipe();
		transform->SetPosition(_playerRespawnPosition);
		transform->SetRotationY(_playerRespawnRotationY);
		rigidbody->SetVelocity(Vector3{});
		rigidbody->ClearForces();
		navMeshController->SetCurrentCellIndex(-1);
		GM_ASSERT(navMeshController->RefreshCellIndex(), "Player 복귀 위치의 Navigation Cell을 찾지 못했습니다.");
		return true;
	}

	void GameplayScene::RespawnPlayer()
	{
		GameObject* player = _player.Get();
		GM_ASSERT_RETURN(player, "리스폰할 Player가 유효하지 않습니다.");
		HealthComponent* healthComponent = player->GetComponent<HealthComponent>();
		PlayerControlComponent* controlComponent = player->GetComponent<PlayerControlComponent>();
		ChiStateMachineComponent* stateMachine = player->GetComponent<ChiStateMachineComponent>();
		GM_ASSERT_RETURN(healthComponent && controlComponent && stateMachine, "Player 리스폰에 필요한 Component가 없습니다.");
		GM_ASSERT_RETURN(ReturnPlayerToRespawnPoint(), "Player를 Respawn Point로 복귀시키지 못했습니다.");

		healthComponent->ClearInvincibility();
		healthComponent->SetHealth(healthComponent->GetMaxHealth());
		stateMachine->ChangeState(ChiStateId::Idle);

		for (Collider3DComponent* collider : _disabledPlayerColliders)
		{
			if (collider)
				collider->SetEnabled(true);
		}
		_disabledPlayerColliders.clear();
		controlComponent->ReleaseControls(this);
		_isPlayerDead = false;
	}

	void GameplayScene::OnUnload()
	{
		_playerDeathConnection.Disconnect();
		_playerDeathAnimationCompletedConnection.Disconnect();
		_disabledPlayerColliders.clear();
		_isPlayerDead = false;
		_player.Reset();
		_dialogComponent = nullptr;
		_announcementWidget = nullptr;
		_playerStatusWidget = nullptr;
		_screenWipeWidget = nullptr;
		_rhythmBarWidget = nullptr;
		_rhythmMeterWidget = nullptr;
		_triggerSystem->Clear();
	}
}
