#pragma once

#include "DialogTypes.h"
#include "Event.h"
#include "UserWidget.h"

namespace gm
{
	class BeatSystem;
	class CanvasPanel;
	class DialogComponent;
	class Image;
	class TextBlock;

	class DialogWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"DialogRoot";

		DialogWidget(const BeatSystem& beatSystem, DialogComponent& dialogComponent);

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		enum class PresentationState
		{
			Hidden,
			Opening,
			Visible,
			Closing,
		};

		void HandleLineChanged(const DialogLineChangedEvent& event);
		void HandleBranchRequested(const DialogBranchRequestedEvent& event);
		void HandleFinished(const DialogFinishedEvent& event);
		void ShowLine(const DialogLineData& line);
		void StartClosing();
		void UpdatePresentation(float deltaTime);
		void UpdatePortraitFrame();
		void ApplyOpenRatio(float ratio);
		void SetPortraitFrame(uint32 frameIndex);

		const BeatSystem&	_beatSystem;
		DialogComponent&	_dialogComponent;
		EventConnection		_lineChangedConnection{};
		EventConnection		_branchRequestedConnection{};
		EventConnection		_finishedConnection{};
		CanvasPanel*		_contentPanel = nullptr;
		Image*				_topBackground = nullptr;
		Image*				_bottomBackground = nullptr;
		Image*				_chiBase = nullptr;
		Image*				_chiMouth = nullptr;
		Image*				_announcerPortrait = nullptr;
		Image*				_saverPortrait = nullptr;
		TextBlock*			_speakerNameText = nullptr;
		TextBlock*			_dialogText = nullptr;
		TextBlock*			_dialogSecondLineText = nullptr;
		DialogPortrait		_currentPortrait = DialogPortrait::Announcer;
		PresentationState	_presentationState = PresentationState::Hidden;
		float				_presentationElapsed = 0.f;
	};
}
