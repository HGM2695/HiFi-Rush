#pragma once

#include "UserWidget.h"

#include <optional>

namespace gm
{
	class BeatSystem;
	class Image;

	enum class GameplayAnnouncementType
	{
		Fight,
		Yeah,

		Count
	};

	class GameplayAnnouncementWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"GameplayAnnouncementRoot";

		explicit GameplayAnnouncementWidget(const BeatSystem& beatSystem);

		void						Play(GameplayAnnouncementType type);
		void						Stop();
		bool						IsPlaying() const { return _isPlaying; }
		GameplayAnnouncementType	GetCurrentType() const { return _currentType; }

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnTick(float deltaTime) override;

	private:
		void					ApplyAnimation(float ratio);

		GameplayAnnouncementType	_currentType = GameplayAnnouncementType::Count;
		const BeatSystem&			_beatSystem;
		Image*						_announcementImage = nullptr;
		std::optional<float>		_startBeat{};
		bool						_isPlaying = false;
	};
}
