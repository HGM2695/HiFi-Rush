#pragma once

#include "Event.h"
#include "UserWidget.h"

#include <optional>

namespace gm
{
	class BeatSystem;
	class ChiStateMachineComponent;
	class Image;

	class BeatHitWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"BeatHitRoot";

		BeatHitWidget(const BeatSystem& beatSystem, ChiStateMachineComponent& stateMachine);

		void	Play(float targetBeat, float approachDurationBeats = 2.f);
		void	ShowSuccess();
		void	ShowMiss();
		void	Hide();
		bool	IsPlaying() const;

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		enum class PresentationState
		{
			Hidden,
			Approaching,
			Success,
			Miss,
		};

		void ResetVisuals();
		void ApplyApproach(float ratio);
		void UpdateApproachGlows(float currentBeat);
		void ApplyResultOpacity(float opacity);
		void BeginResult(PresentationState resultState);

		const BeatSystem&		_beatSystem;
		ChiStateMachineComponent& _stateMachine;
		EventConnection			_beatHitStartedConnection{};
		EventConnection			_beatHitResultConnection{};
		Image*					_insideCircle = nullptr;
		Image*					_insideGlow = nullptr;
		Image*					_outsideGlow = nullptr;
		Image*					_outsideCircle = nullptr;
		Image*					_lightCircle = nullptr;
		Image*					_lightLine = nullptr;
		Image*					_thorn = nullptr;
		PresentationState		_state = PresentationState::Hidden;
		std::optional<float>	_resultStartBeat{};
		float					_approachStartBeat = 0.f;
		float					_targetBeat = 0.f;
		float					_approachDurationBeats = 2.f;
	};
}
