#include "LoadingScreenWidget.h"
#include "Application.h"
#include "Border.h"
#include "CanvasPanel.h"
#include "Image.h"
#include "WidgetTween.h"

#include <algorithm>
#include <array>

namespace gm
{
	namespace
	{
		constexpr wchar_t LoadingScreenTextureKey[] = L"T_loading_screen_808";
		constexpr wchar_t LoadingNote1TextureKey[] = L"T_loading_screen_note_1";
		constexpr wchar_t LoadingNote2TextureKey[] = L"T_loading_screen_note_2";
		constexpr wchar_t LoadingNote3TextureKey[] = L"T_loading_screen_note_3";
		constexpr float NoteRevealInterval = 0.18f;
		constexpr float NotePopDuration = 0.16f;
		constexpr float NoteHideTime = 0.9f;
		constexpr float NoteAnimationCycle = 1.2f;
		constexpr std::array<uint32, 3> NoteRevealOrder{ 0, 2, 1 };
	}

	std::unique_ptr<Widget> LoadingScreenWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(L"LoadingScreenRoot");
		root->SetSizeRule(WidgetSizeRule::FillParent);

		const float screenWidth = static_cast<float>(APPLICATION.GetWidth());
		const float screenHeight = static_cast<float>(APPLICATION.GetHeight());

		Border* background = root->AddNamedChild<Border>(L"Background");
		background->SetPosition({ screenWidth * 0.5f, screenHeight * 0.5f });
		background->SetSize({ screenWidth, screenHeight });
		background->SetBackgroundColor(Colors::Black);
		background->SetBorderThickness(0.f);

		const Vector2 catPosition{ 1392.4f, 705.3f };
		const Vector2 catSize{ 306.4f, 323.f };

		Image* catImage = root->AddNamedChild<Image>(L"Cat", LoadingScreenTextureKey);
		catImage->SetPosition(catPosition);
		catImage->SetSize(catSize);

		const float noteSize = std::min(88.f, screenHeight * 0.1f);
		const std::array<const wchar_t*, 3> noteKeys{ LoadingNote1TextureKey, LoadingNote2TextureKey, LoadingNote3TextureKey };
		const std::array<Vector2, 3> notePositions
		{
			Vector2{ 1185.7f, 652.9f },
			Vector2{ 1024.7f, 662.7f },
			Vector2{ 1132.2f, 595.3f },
		};
		std::array<Image*, 3> noteImages{};

		for (uint32 noteIndex = 0; noteIndex < noteKeys.size(); ++noteIndex)
		{
			Image* noteImage = root->AddNamedChild<Image>(L"Note" + std::to_wstring(noteIndex + 1), noteKeys[noteIndex]);
			noteImage->SetPosition(notePositions[noteIndex]);
			noteImage->SetSize({ noteSize, noteSize });
			noteImages[noteIndex] = noteImage;
		}

		for (uint32 revealIndex = 0; revealIndex < NoteRevealOrder.size(); ++revealIndex)
		{
			WidgetSizeTweenDesc desc{};
			desc.to = Vector2{ noteSize, noteSize };
			desc.startDelay = static_cast<float>(revealIndex) * NoteRevealInterval;
			desc.forwardDuration = NotePopDuration;
			desc.holdDuration = NoteHideTime - desc.startDelay - desc.forwardDuration;
			desc.repeatDelay = NoteAnimationCycle - NoteHideTime;
			desc.ease = WidgetTweenEase::OutBack;
			desc.repeat = true;
			noteImages[NoteRevealOrder[revealIndex]]->AddTween<WidgetSizeTween>(desc);
		}

		return root;
	}
}
