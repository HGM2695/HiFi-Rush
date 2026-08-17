#pragma once

#include "Event.h"
#include "UserWidget.h"

#include <array>

namespace gm
{
	class BeatSystem;
	class ChiStateMachineComponent;
	class HealthComponent;
	class Image;
	class ReverbComponent;
	struct HealthChangedEvent;
	struct ReverbChangedEvent;
	struct RhythmJudgeResult;

	class PlayerStatusWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"PlayerStatusRoot";

		PlayerStatusWidget(const BeatSystem& beatSystem, HealthComponent& healthComponent, ReverbComponent& reverbComponent, ChiStateMachineComponent& stateMachine);

		void PlayBeatFeedback();

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		struct AfterImageEntry
		{
			Image*	image = nullptr;
			Vector2 targetOffset{};
		};

		void HandleHealthChanged(const HealthChangedEvent& event);
		void HandleReverbChanged(const ReverbChangedEvent& event);
		void HandleRhythmInputJudged(const RhythmJudgeResult& result);
		void UpdateStatus(float deltaTime);
		void UpdateShadow();
		void UpdateThunder();
		void UpdateAfterImages();
		void SetImageGeometry(Image& image, const Vector2& center, const Vector2& size) const;

	private:
		static constexpr uint32							AfterImageCount = 4;
		std::array<AfterImageEntry, AfterImageCount>	_afterImages{};

		Image*	_backgroundImage = nullptr;
		Image*	_fullBackgroundImage = nullptr;
		Image*	_shadowImage = nullptr;
		Image*	_healthImage = nullptr;
		Image*	_reverbImage = nullptr;
		Image*	_fullReverbImage = nullptr;
		Image*	_thunderImage = nullptr;

		const BeatSystem&	_beatSystem;
		HealthComponent&	_healthComponent;
		ReverbComponent&	_reverbComponent;
		ChiStateMachineComponent& _stateMachine;
		EventConnection		_healthChangedConnection;
		EventConnection		_reverbChangedConnection;
		EventConnection		_rhythmInputJudgedConnection;

		float	_healthRatio = 1.f;
		float	_targetHealthRatio = 1.f;
		float	_reverbRatio = 0.f;
		float	_targetReverbRatio = 0.f;
		float	_afterImageStartBeat = 0.f;
		uint32	_thunderFrameIndex = 0;
		bool	_isAfterImageActive = false;
	};
}
