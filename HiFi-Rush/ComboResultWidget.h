#pragma once

#include "Event.h"
#include "UserWidget.h"

namespace gm
{
	class ChiStateMachineComponent;
	class Image;
	struct RhythmJudgeResult;

	class ComboResultWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"ComboRoot";

		explicit ComboResultWidget(ChiStateMachineComponent& stateMachine);

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		void					HandleRhythmActionStarted(const RhythmJudgeResult& result);
		void					Play(uint32 comboIndex);
		void					Stop();
		void					ApplyAnimation(float ratio);

		ChiStateMachineComponent&	_stateMachine;
		EventConnection				_rhythmActionConnection{};
		Image*						_comboImage = nullptr;
		uint32						_comboIndex = 0;
		float						_elapsedTime = 0.f;
		bool						_isPlaying = false;
	};
}
