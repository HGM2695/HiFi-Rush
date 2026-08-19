#include "AttackGuideWidget.h"

#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "ChiStateTypes.h"
#include "HiFiRushFonts.h"
#include "Image.h"
#include "TextBlock.h"

#include <array>

namespace gm
{
	namespace
	{
		struct AttackGuideVisualInfo
		{
			const wchar_t*	textureKey = nullptr;
			const wchar_t*	title = nullptr;
			const wchar_t*	summary = nullptr;
			const wchar_t*	detail = nullptr;
			const wchar_t*	detailSecondLine = nullptr;
			const wchar_t*	attackName = nullptr;
			float			summaryFontSize = 0.f;
		};

		constexpr std::array<AttackGuideVisualInfo, static_cast<uint32>(RhythmTutorialType::Count)> AttackGuideVisualInfos =
		{{
			{ L"WeakAttack", L"비트를 들어 보세요!", L"약공격 1비트", L"비트에 맞춰 마우스를 눌러", L"더 많은 피해를 주고 높은 점수를 얻으세요.", L"약공격", 18.67f },
			{ L"StrongAttack", L"더욱 길고 강하게!", L"표적을 맞히는 데 2비트가 필요합니다.", L"공격이 적중할 때 다시 공격 버튼을 눌러", L"리듬에 맞춰 콤보를 이어 나가세요.", L"강공격", 16.33f },
		}};

		constexpr Vector2 BackgroundCenter{ 863.75f, 365.625f };
		constexpr Vector2 BackgroundSize{ 1425.f, 303.75f };
		constexpr Vector2 TitlePosition{ 1251.875f, 272.5f };
		constexpr Vector2 SummaryPosition{ 1251.25f, 365.625f };
		constexpr Vector2 DetailPosition{ 1251.25f, 409.375f };
		constexpr Vector2 DetailSecondLinePosition{ 1251.25f, 429.375f };
		constexpr Vector2 AttackNamePosition{ 178.125f, 368.125f };
		constexpr float TitleFontSize = 21.f;
		constexpr float DetailFontSize = 16.33f;
		constexpr float AttackNameFontSize = 18.67f;

		void ConfigureGuideText(TextBlock& text, const Vector2& position, float fontSize)
		{
			text.SetPosition(position);
			text.SetSize(fontSize);
			text.SetColor(Colors::Black);
			text.SetFont(HiFiRushFont::NanumSquareRoundExtraBold);
		}
	}

	AttackGuideWidget::AttackGuideWidget(ChiStateMachineComponent& stateMachine)
		: _stateMachine(stateMachine)
	{
	}

	void AttackGuideWidget::Show(RhythmTutorialType type)
	{
		GM_ASSERT_RETURN(type < RhythmTutorialType::Count, "지원하지 않는 Attack Guide Type입니다.");

		_activeType = type;
		const AttackGuideVisualInfo& info = AttackGuideVisualInfos[static_cast<uint32>(type)];
		_background->SetTexture(info.textureKey);
		_titleText->SetText(info.title);
		_summaryText->SetText(info.summary);
		_summaryText->SetSize(info.summaryFontSize);
		_detailText->SetText(info.detail);
		_detailSecondLineText->SetText(info.detailSecondLine);
		_attackNameText->SetText(info.attackName);
		_contentPanel->SetVisible(true);
	}

	void AttackGuideWidget::Hide()
	{
		_activeType = RhythmTutorialType::Count;
		_contentPanel->SetVisible(false);
	}

	std::unique_ptr<Widget> AttackGuideWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_contentPanel = root->AddNamedChild<CanvasPanel>(L"AttackGuideContent");
		_background = _contentPanel->AddNamedChild<Image>(L"AttackGuideBackground", L"WeakAttack");
		_background->SetGeometry(BackgroundCenter, BackgroundSize);

		_titleText = _contentPanel->AddNamedChild<TextBlock>(L"AttackGuideTitle");
		_summaryText = _contentPanel->AddNamedChild<TextBlock>(L"AttackGuideSummary");
		_detailText = _contentPanel->AddNamedChild<TextBlock>(L"AttackGuideDetail");
		_detailSecondLineText = _contentPanel->AddNamedChild<TextBlock>(L"AttackGuideDetailSecondLine");
		_attackNameText = _contentPanel->AddNamedChild<TextBlock>(L"AttackGuideAttackName");
		ConfigureGuideText(*_titleText, TitlePosition, TitleFontSize);
		ConfigureGuideText(*_summaryText, SummaryPosition, AttackNameFontSize);
		ConfigureGuideText(*_detailText, DetailPosition, DetailFontSize);
		ConfigureGuideText(*_detailSecondLineText, DetailSecondLinePosition, DetailFontSize);
		ConfigureGuideText(*_attackNameText, AttackNamePosition, AttackNameFontSize);
		_contentPanel->SetVisible(false);
		return root;
	}

	void AttackGuideWidget::OnInitialize()
	{
		_stateMachine.OnRhythmActionStarted.Subscribe(_rhythmActionConnection,
			[this](const RhythmJudgeResult& result)
			{
				HandleRhythmActionStarted(result);
			});
	}

	void AttackGuideWidget::HandleRhythmActionStarted(const RhythmJudgeResult&)
	{
		const ChiStateId stateId = _stateMachine.GetCurrentStateId();
		const bool hasCompletedWeakGuide = _activeType == RhythmTutorialType::Weak && stateId == ChiStateId::AttackWeak3;
		const bool hasCompletedStrongGuide = _activeType == RhythmTutorialType::Strong && stateId == ChiStateId::AttackStrong2;
		if (hasCompletedWeakGuide == false && hasCompletedStrongGuide == false)
			return;

		AttackGuideCompletedEvent event{};
		event.type = _activeType;
		Hide();
		OnCompleted.Publish(event);
	}
}
