#pragma once

#include "Scene.h"
#include "WeakGameObjectPtr.h"

#include <memory>
#include <string>
#include <vector>

namespace gm
{
	struct PlayerSpawnDesc;
	struct MonsterSpawnResult;
	class DialogComponent;
	class TriggerSystem;
	class GameplayAnnouncementWidget;
	class PlayerStatusWidget;
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

protected:
		bool InitializeMap(const std::wstring& mapResourceKey);
		bool InitializeMap(const std::wstring& mapResourceKey, std::vector<MonsterSpawnResult>& outMonsterSpawnResults);
		bool InitializePlayer(const PlayerSpawnDesc& desc);
		void InitializeGameplayUI();
		void OnUnload() override;

	private:
		std::unique_ptr<TriggerSystem>			_triggerSystem;
		WeakGameObjectPtr						_player{};
		DialogComponent*						_dialogComponent = nullptr;
		GameplayAnnouncementWidget*				_announcementWidget = nullptr;
		PlayerStatusWidget*						_playerStatusWidget = nullptr;
		RhythmMeterWidget*						_rhythmMeterWidget = nullptr;
	};
}
