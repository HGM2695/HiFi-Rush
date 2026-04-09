#include "MainHUDWidget.h"
#include "Border.h"
#include "CanvasPanel.h"
#include "TextBlock.h"

namespace gm
{
	std::unique_ptr<Widget> MainHUDWidget::BuildWidgetTree()
	{
		auto root = std::make_unique<CanvasPanel>();

		Border* panel = root->AddChild<Border>();
		panel->SetPosition({ 20.f, 20.f });
		panel->SetSize({ 320.f, 110.f });
		panel->SetBackgroundColor({ 24, 24, 24, 220 });
		panel->SetOutlineColor({ 90, 180, 255, 255 });

		/*
		Image* panel = root->AddChild<Image>();
		panel->SetPosition({ 20.f, 20.f });
		panel->SetSize({ 320.f, 110.f });
		panel->SetTextureByName(L"OrangeMushroom");
		*/

		TextBlock* titleText = panel->AddChild<TextBlock>();
		titleText->SetPosition({ 16.f, 14.f });
		titleText->SetText(L"Main HUD");
		titleText->SetColor(Color::Cyan());

		TextBlock* bodyText = panel->AddChild<TextBlock>();
		bodyText->SetPosition({ 16.f, 44.f });
		bodyText->SetText(L"Move : Arrow Key  |  Jump : T");
		bodyText->SetColor(Color::White());

		TextBlock* hintText = panel->AddChild<TextBlock>();
		hintText->SetPosition({ 16.f, 72.f });
		hintText->SetText(L"Unreal-style UserWidget prototype");
		hintText->SetColor(Color::Yellow());

		return root;
	}
}
