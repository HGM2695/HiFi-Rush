#pragma once

#include "RhythmTutorialTypes.h"
#include "Event.h"
#include "UserWidget.h"

namespace gm
{
	class CanvasPanel;
	class ChiStateMachineComponent;
	class Image;
	class TextBlock;
	struct RhythmJudgeResult;

	struct AttackGuideCompletedEvent final : EventType
	{
		RhythmTutorialType type = RhythmTutorialType::Count;
	};

	class AttackGuideWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"AttackGuideRoot";

		explicit AttackGuideWidget(ChiStateMachineComponent& stateMachine);

		void Show(RhythmTutorialType type);
		void Hide();

		EventPublisher<AttackGuideWidget, AttackGuideCompletedEvent> OnCompleted;

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void OnInitialize() override;

	private:
		void HandleRhythmActionStarted(const RhythmJudgeResult& result);

		ChiStateMachineComponent&	_stateMachine;
		EventConnection				_rhythmActionConnection{};
		CanvasPanel*				_contentPanel = nullptr;
		Image*						_background = nullptr;
		TextBlock*					_titleText = nullptr;
		TextBlock*					_summaryText = nullptr;
		TextBlock*					_detailText = nullptr;
		TextBlock*					_detailSecondLineText = nullptr;
		TextBlock*					_attackNameText = nullptr;
		RhythmTutorialType			_activeType = RhythmTutorialType::Count;
	};
}
