#pragma once

#include "Event.h"
#include "UserWidget.h"

#include <array>
#include <optional>

namespace gm
{
	class BeatSystem;
	class ChiStateMachineComponent;
	class Image;
	struct ChiStateChangedEvent;

	class HibikiWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"HibikiRoot";

		HibikiWidget(const BeatSystem& beatSystem, ChiStateMachineComponent& stateMachine);

		void Play();
		void Stop();
		bool IsPlaying() const { return _isPlaying; }

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		void HandleStateChanged(const ChiStateChangedEvent& event);
		void ResetPresentation();
		void MoveImage(Image& image, const Vector2& targetCenter, float interpolationRatio);
		void ActivateImage(Image& image);

		const BeatSystem&			_beatSystem;
		ChiStateMachineComponent&	_stateMachine;
		EventConnection				_stateChangedConnection{};
		Image*						_backgroundImage = nullptr;
		Image*						_originalImage = nullptr;
		Image*						_fontImage = nullptr;
		std::array<Image*, 3>		_afterImages{};
		std::optional<float>		_startBeat{};
		float						_elapsedTime = 0.f;
		bool						_isPlaying = false;
	};
}
