#pragma once

#include "Scene.h"
#include "WeakGameObjectPtr.h"

#include <memory>
#include <string>

namespace gm
{
	struct PlayerSpawnDesc;
	class TriggerSequenceSystem;
	class DialogComponent;
	class GameplayAnnouncementWidget;
	class PlayerStatusWidget;
	class RhythmMeterWidget;
	enum class GameplayAnnouncementType;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene();
		~GameplayScene() override;

		TriggerSequenceSystem&			GetTriggerSequenceSystem();
		const TriggerSequenceSystem&	GetTriggerSequenceSystem() const;
		const WeakGameObjectPtr&		GetPlayer() const { return _player; }
		DialogComponent&				GetDialogComponent();
		const DialogComponent&			GetDialogComponent() const;
		bool							PlayDialogSequence(const std::wstring& sequenceId);
		bool							SelectDialogBranch(const std::wstring& branchKey);
		void PlayAnnouncement(GameplayAnnouncementType type);
		void SetGameplayStatusUIVisible(bool isVisible);

	protected:
		bool InitializeMap(const std::wstring& mapResourceKey);
		bool InitializePlayer(const PlayerSpawnDesc& desc);
		void InitializeGameplayUI();
		void OnUnload() override;

	private:
		std::unique_ptr<TriggerSequenceSystem>	_triggerSequenceSystem;
		WeakGameObjectPtr						_player{};
		GameplayAnnouncementWidget*			_announcementWidget = nullptr;
		DialogComponent*						_dialogComponent = nullptr;
		PlayerStatusWidget*					_playerStatusWidget = nullptr;
		RhythmMeterWidget*					_rhythmMeterWidget = nullptr;
	};
}
