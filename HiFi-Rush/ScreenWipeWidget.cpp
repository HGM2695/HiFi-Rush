#include "ScreenWipeWidget.h"

#include "CanvasPanel.h"
#include "Image.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		constexpr wchar_t TopTextureKey[] = L"Dead0";
		constexpr wchar_t BottomTextureKey[] = L"Dead1";
		constexpr Vector2 PanelSize{ 2458.5f, 544.f };
		constexpr Vector2 TopStartPosition{ 690.75f, 270.f };
		constexpr Vector2 BottomStartPosition{ 1229.25f, 810.f };
		constexpr float AnimationDuration = 1.f;
	}

	void ScreenWipeWidget::PlayOpen()
	{
		GM_ASSERT_RETURN(_topImage && _bottomImage, "Screen Wipe Widget이 초기화되지 않았습니다.");
		_elapsedTime = 0.f;
		_state = State::Opening;
		ApplyOpenRatio(0.f);
		SetVisible(true);
	}

	void ScreenWipeWidget::PlayCover()
	{
		GM_ASSERT_RETURN(_topImage && _bottomImage, "Screen Wipe Widget이 초기화되지 않았습니다.");
		_elapsedTime = 0.f;
		_state = State::Covering;
		ApplyOpenRatio(1.f);
		SetVisible(true);
	}

	void ScreenWipeWidget::Hide()
	{
		_elapsedTime = 0.f;
		_state = State::Hidden;
		SetVisible(false);
	}

	std::unique_ptr<Widget> ScreenWipeWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		root->SetSizeRule(WidgetSizeRule::FillParent);
		_topImage = root->AddNamedChild<Image>(L"ScreenWipeTop", TopTextureKey);
		_bottomImage = root->AddNamedChild<Image>(L"ScreenWipeBottom", BottomTextureKey);
		ApplyOpenRatio(0.f);
		return root;
	}

	void ScreenWipeWidget::OnInitialize()
	{
		Hide();
	}

	void ScreenWipeWidget::OnTick(float deltaTime)
	{
		if (_state != State::Opening && _state != State::Covering)
			return;

		_elapsedTime += std::max(0.f, deltaTime);
		const float ratio = std::clamp(_elapsedTime / AnimationDuration, 0.f, 1.f);
		ApplyOpenRatio(_state == State::Opening ? ratio : 1.f - ratio);
		if (ratio < 1.f)
			return;

		if (_state == State::Opening)
		{
			Hide();
			return;
		}

		_state = State::Covered;
	}

	void ScreenWipeWidget::ApplyOpenRatio(float ratio)
	{
		const float moveDistance = PanelSize.x * ratio;
		_topImage->SetGeometry({ TopStartPosition.x + moveDistance, TopStartPosition.y }, PanelSize);
		_bottomImage->SetGeometry({ BottomStartPosition.x - moveDistance, BottomStartPosition.y }, PanelSize);
	}
}
