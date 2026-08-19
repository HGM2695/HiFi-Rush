#pragma once

#include "Event.h"
#include "Scene.h"
#include "WeakGameObjectPtr.h"

#include <memory>
#include <string>
#include <vector>

namespace gm
{
	struct PlayerSpawnDesc;
	struct MonsterSpawnResult;
	struct HitEvent;
	struct PlayerDeathAnimationCompletedEvent;
	class Collider3DComponent;
	class DialogComponent;
	class TriggerSystem;
	class GameplayAnnouncementWidget;
	class PlayerStatusWidget;
	class RespawnWipeWidget;
	class RhythmMeterWidget;
	enum class GameplayAnnouncementType;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene();
		~GameplayScene() override;

		TriggerSystem&					GetTriggerSystem();
		const TriggerSystem&			GetTriggerSystem() const;
		const WeakGameObjectPtr&		GetPlayer() const { return _player; }
		DialogComponent&				GetDialogComponent();
		const DialogComponent&			GetDialogComponent() const;
		bool							PlayDialogSequence(const std::wstring& sequenceId);
		bool							SelectDialogBranch(const std::wstring& branchKey);
		void							PlayAnnouncement(GameplayAnnouncementType type);
		void							SetGameplayStatusUIVisible(bool isVisible);
		void							SetPlayerRespawnPoint(const Vector3& position, float rotationY);
		void							HandlePlayerFall(int32 damage);

protected:
		bool InitializeMap(const std::wstring& mapResourceKey);
		bool InitializeMap(const std::wstring& mapResourceKey, std::vector<MonsterSpawnResult>& outMonsterSpawnResults);
		bool InitializePlayer(const PlayerSpawnDesc& desc);
		void InitializeGameplayUI();
		void OnUnload() override;

	private:
		void HandlePlayerDeath(const HitEvent& event);
		void HandlePlayerDeathAnimationCompleted(const PlayerDeathAnimationCompletedEvent& event);
		bool ReturnPlayerToRespawnPoint();
		void RespawnPlayer();

	private:
		std::unique_ptr<TriggerSystem>			_triggerSystem;
		WeakGameObjectPtr						_player{};
		DialogComponent*						_dialogComponent = nullptr;
		GameplayAnnouncementWidget*				_announcementWidget = nullptr;
		PlayerStatusWidget*						_playerStatusWidget = nullptr;
		RhythmMeterWidget*						_rhythmMeterWidget = nullptr;

		Vector3									_playerRespawnPosition{};
		float									_playerRespawnRotationY = 0.f;
		std::vector<Collider3DComponent*>		_disabledPlayerColliders{};
		EventConnection							_playerDeathConnection{};
		EventConnection							_playerDeathAnimationCompletedConnection{};
		RespawnWipeWidget*						_respawnWipeWidget = nullptr;
		bool									_isPlayerDead = false;
	};
}
