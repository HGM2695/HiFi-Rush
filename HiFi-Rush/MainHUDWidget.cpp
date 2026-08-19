#include "MainHUDWidget.h"
#include "Border.h"
#include "CanvasPanel.h"
#include "TextBlock.h"
#include "Image.h"

namespace gm
{
	std::unique_ptr<Widget> MainHUDWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(L"Root");

		Border* border = root->AddNamedChild<Border>(L"MainPanel");
		border->SetGeometry({ 0.f, -125.f }, { 460.f, 110.f });
		border->SetBackgroundColor(ColorFromRGBA(24, 24, 24, 220));
		border->SetOutlineColor(Colors::Red);
		border->SetBorderThickness(2.f);

		TextBlock* titleText = border->AddNamedChild<TextBlock>(L"TitleText");
		titleText->SetHorizonAlign(TextHorizontalAlignment::Center);
		titleText->SetVerticalAlign(TextVerticalAlignment::Center);
		titleText->SetPosition({ 0.f, -35.f });
		titleText->SetText(L"Main HUD");
		titleText->SetColor(Colors::Cyan);

		TextBlock* bodyText = border->AddNamedChild<TextBlock>(L"BodyText");
		bodyText->SetHorizonAlign(TextHorizontalAlignment::Center);
		bodyText->SetVerticalAlign(TextVerticalAlignment::Center);
		bodyText->SetPosition({ 0.f, -5.f });
		bodyText->SetText(L"Move : Arrow Key  |  Jump : C");
		bodyText->SetColor(Colors::White);

		TextBlock* hintText = border->AddNamedChild<TextBlock>(L"HintText");
		hintText->SetHorizonAlign(TextHorizontalAlignment::Center);
		hintText->SetVerticalAlign(TextVerticalAlignment::Center);
		hintText->SetPosition({ 0.f, 25.f });
		hintText->SetText(L"Unreal-style UserWidget prototype");
		hintText->SetColor(Colors::Yellow);

		return root;
	}
}
