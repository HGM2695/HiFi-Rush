#include "DialogWidget.h"

#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "DialogComponent.h"
#include "HiFiRushFonts.h"
#include "Image.h"
#include "TextBlock.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr Vector2 TopBackgroundCenter{ 800.f, 79.375f };
		constexpr Vector2 TopBackgroundHiddenCenter{ 800.f, -79.375f };
		constexpr Vector2 TopBackgroundSize{ 1600.f, 158.75f };
		constexpr Vector2 BottomBackgroundCenter{ 800.f, 820.625f };
		constexpr Vector2 BottomBackgroundHiddenCenter{ 800.f, 979.375f };
		constexpr Vector2 BottomBackgroundSize{ 1600.f, 158.75f };
		constexpr Vector2 WindowCenter{ 886.25f, 775.f };
		constexpr Vector2 WindowSize{ 1000.f, 150.f };
		constexpr Vector2 ChiBaseCenter{ 268.75f, 728.125f };
		constexpr Vector2 ChiBaseSize{ 537.5f, 343.75f };
		constexpr Vector2 ChiMouthCenter{ 222.5f, 790.625f };
		constexpr Vector2 ChiMouthSize{ 65.f, 43.75f };
		constexpr Vector2 AnnouncerCenter{ 215.f, 724.375f };
		constexpr Vector2 AnnouncerSize{ 312.5f, 343.75f };
		constexpr Vector2 SaverCenter{ 239.375f, 742.5f };
		constexpr Vector2 SaverSize{ 501.25f, 312.5f };
		constexpr Vector2 SpeakerNamePosition{ 501.25f, 705.f };
		constexpr Vector2 DialogSingleLinePosition{ 472.5f, 776.25f };
		constexpr Vector2 DialogFirstLinePosition{ 472.5f, 760.f };
		constexpr Vector2 DialogSecondLinePosition{ 472.5f, 792.5f };
		constexpr float SpeakerNameFontSize = 18.67f;
		constexpr float DialogTextFontSize = 25.67f;
		constexpr float DefaultPresentationDuration = 0.2f;
		constexpr float MediumVoiceAmplitude = 0.1f;
		constexpr float LargeVoiceAmplitude = 0.3f;
		constexpr std::array<const wchar_t*, 3> ChiMouthTextureKeys
		{
			L"T_talk_chai_pleasure_mouth_00",
			L"T_talk_chai_pleasure_mouth_01",
			L"T_talk_chai_pleasure_mouth_02",
		};
		constexpr std::array<const wchar_t*, 3> AnnouncerTextureKeys
		{
			L"T_talk_speaker_00",
			L"T_talk_speaker_01",
			L"T_talk_speaker_02",
		};
		constexpr std::array<const wchar_t*, 3> SaverTextureKeys
		{
			L"Saver0",
			L"Saver1",
			L"Saver2",
		};
	}

	DialogWidget::DialogWidget(const BeatSystem& beatSystem, DialogComponent& dialogComponent)
		: _beatSystem(beatSystem), _dialogComponent(dialogComponent)
	{
	}

	std::unique_ptr<Widget> DialogWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_topBackground = root->AddNamedChild<Image>(L"DialogTopBackground", L"BackGround_Top");
		_bottomBackground = root->AddNamedChild<Image>(L"DialogBottomBackground", L"BackGround_Bottom");
		_topBackground->SetGeometry(TopBackgroundHiddenCenter, TopBackgroundSize);
		_bottomBackground->SetGeometry(BottomBackgroundHiddenCenter, BottomBackgroundSize);
		_topBackground->SetVisible(false);
		_bottomBackground->SetVisible(false);

		_contentPanel = root->AddNamedChild<CanvasPanel>(L"DialogContent");
		_chiBase = _contentPanel->AddNamedChild<Image>(L"ChiPortrait", L"T_talk_chai_00_base");
		_chiMouth = _contentPanel->AddNamedChild<Image>(L"ChiMouth", ChiMouthTextureKeys[0]);
		_announcerPortrait = _contentPanel->AddNamedChild<Image>(L"AnnouncerPortrait", AnnouncerTextureKeys[0]);
		_saverPortrait = _contentPanel->AddNamedChild<Image>(L"SaverPortrait", SaverTextureKeys[0]);
		_chiBase->SetGeometry(ChiBaseCenter, ChiBaseSize);
		_chiMouth->SetGeometry(ChiMouthCenter, ChiMouthSize);
		_announcerPortrait->SetGeometry(AnnouncerCenter, AnnouncerSize);
		_saverPortrait->SetGeometry(SaverCenter, SaverSize);

		Image* window = _contentPanel->AddNamedChild<Image>(L"DialogWindow", L"Window2");
		window->SetGeometry(WindowCenter, WindowSize);

		_speakerNameText = _contentPanel->AddNamedChild<TextBlock>(L"DialogSpeakerName");
		_speakerNameText->SetPosition(SpeakerNamePosition);
		_speakerNameText->SetSize(SpeakerNameFontSize);
		_speakerNameText->SetColor(Colors::White);
		_speakerNameText->SetFont(HiFiRushFont::NanumSquareRoundExtraBold);

		_dialogText = _contentPanel->AddNamedChild<TextBlock>(L"DialogText");
		_dialogText->SetPosition(DialogSingleLinePosition);
		_dialogText->SetSize(DialogTextFontSize);
		_dialogText->SetColor(Colors::Black);
		_dialogText->SetFont(HiFiRushFont::NanumSquareRoundExtraBold);

		_dialogSecondLineText = _contentPanel->AddNamedChild<TextBlock>(L"DialogSecondLineText");
		_dialogSecondLineText->SetPosition(DialogSecondLinePosition);
		_dialogSecondLineText->SetSize(DialogTextFontSize);
		_dialogSecondLineText->SetColor(Colors::Black);
		_dialogSecondLineText->SetFont(HiFiRushFont::NanumSquareRoundExtraBold);

		_contentPanel->SetVisible(false);
		return root;
	}

	void DialogWidget::OnInitialize()
	{
		_dialogComponent.OnLineChanged.Subscribe(_lineChangedConnection,
			[this](const DialogLineChangedEvent& event)
			{
				HandleLineChanged(event);
			});
		_dialogComponent.OnBranchRequested.Subscribe(_branchRequestedConnection,
			[this](const DialogBranchRequestedEvent& event)
			{
				HandleBranchRequested(event);
			});
		_dialogComponent.OnFinished.Subscribe(_finishedConnection,
			[this](const DialogFinishedEvent& event)
			{
				HandleFinished(event);
			});
	}

	void DialogWidget::OnTick(float deltaTime)
	{
		UpdatePresentation(deltaTime);
		UpdatePortraitFrame();
	}

	void DialogWidget::HandleLineChanged(const DialogLineChangedEvent& event)
	{
		GM_ASSERT_RETURN(event.line, "DialogWidget에 유효하지 않은 대사가 전달되었습니다.");
		ShowLine(*event.line);
	}

	void DialogWidget::HandleBranchRequested(const DialogBranchRequestedEvent&)
	{
		StartClosing();
	}

	void DialogWidget::HandleFinished(const DialogFinishedEvent&)
	{
		StartClosing();
	}

	void DialogWidget::ShowLine(const DialogLineData& line)
	{
		_currentPortrait = line.portrait;
		_speakerNameText->SetText(line.speakerName);
		const size_t lineBreakIndex = line.text.find(L'\n');
		if (lineBreakIndex == std::wstring::npos)
		{
			_dialogText->SetPosition(DialogSingleLinePosition);
			_dialogText->SetText(line.text);
			_dialogSecondLineText->SetText(L"");
		}
		else
		{
			_dialogText->SetPosition(DialogFirstLinePosition);
			_dialogText->SetText(line.text.substr(0, lineBreakIndex));
			_dialogSecondLineText->SetText(line.text.substr(lineBreakIndex + 1));
		}

		_chiBase->SetVisible(line.portrait == DialogPortrait::Chi);
		_chiMouth->SetVisible(line.portrait == DialogPortrait::Chi);
		_announcerPortrait->SetVisible(line.portrait == DialogPortrait::Announcer);
		_saverPortrait->SetVisible(line.portrait == DialogPortrait::Saver);
		SetPortraitFrame(0);
		_contentPanel->SetVisible(true);

		if (_presentationState == PresentationState::Hidden || _presentationState == PresentationState::Closing)
		{
			_presentationState = PresentationState::Opening;
			_presentationElapsed = 0.f;
			_topBackground->SetVisible(true);
			_bottomBackground->SetVisible(true);
			ApplyOpenRatio(0.f);
		}
	}

	void DialogWidget::StartClosing()
	{
		if (_presentationState == PresentationState::Hidden || _presentationState == PresentationState::Closing)
			return;

		_contentPanel->SetVisible(false);
		_presentationState = PresentationState::Closing;
		_presentationElapsed = 0.f;
	}

	void DialogWidget::UpdatePresentation(float deltaTime)
	{
		if (_presentationState != PresentationState::Opening && _presentationState != PresentationState::Closing)
			return;

		const float presentationDuration = _beatSystem.HasPlaybackTime() ? std::max(_beatSystem.GetSecondsPerBeat() / 3.f, 0.01f) : DefaultPresentationDuration;
		_presentationElapsed += std::max(0.f, deltaTime);
		const float ratio = std::clamp(_presentationElapsed / presentationDuration, 0.f, 1.f);
		ApplyOpenRatio(_presentationState == PresentationState::Opening ? ratio : 1.f - ratio);

		if (ratio < 1.f)
			return;

		if (_presentationState == PresentationState::Opening)
		{
			_presentationState = PresentationState::Visible;
			return;
		}

		_presentationState = PresentationState::Hidden;
		_topBackground->SetVisible(false);
		_bottomBackground->SetVisible(false);
	}

	void DialogWidget::UpdatePortraitFrame()
	{
		if (_contentPanel->IsVisible() == false)
			return;

		const float voiceAmplitude = _dialogComponent.GetVoiceSpectrumAmplitude();
		uint32 frameIndex = 0;
		if (voiceAmplitude > LargeVoiceAmplitude)
			frameIndex = 2;
		else if (voiceAmplitude > MediumVoiceAmplitude)
			frameIndex = 1;

		SetPortraitFrame(frameIndex);
	}

	void DialogWidget::ApplyOpenRatio(float ratio)
	{
		const float easedRatio = 1.f - std::pow(1.f - std::clamp(ratio, 0.f, 1.f), 3.f);
		_topBackground->SetPosition(TopBackgroundHiddenCenter + (TopBackgroundCenter - TopBackgroundHiddenCenter) * easedRatio);
		_bottomBackground->SetPosition(BottomBackgroundHiddenCenter + (BottomBackgroundCenter - BottomBackgroundHiddenCenter) * easedRatio);
	}

	void DialogWidget::SetPortraitFrame(uint32 frameIndex)
	{
		frameIndex = std::min(frameIndex, 2u);
		switch (_currentPortrait)
		{
		case DialogPortrait::Chi:
			_chiMouth->SetTexture(ChiMouthTextureKeys[frameIndex]);
			break;
		case DialogPortrait::Announcer:
			_announcerPortrait->SetTexture(AnnouncerTextureKeys[frameIndex]);
			break;
		case DialogPortrait::Saver:
			_saverPortrait->SetTexture(SaverTextureKeys[frameIndex]);
			break;
		}
	}
}
