#include "RespawnWipeWidget.h"

#include "CanvasPanel.h"
#include "Image.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		constexpr wchar_t TopTextureKey[] = L"Dead0";
		constexpr wchar_t BottomTextureKey[] = L"Dead1";
		constexpr Vector2 PanelSize{ 2458.5f, 543.f };
		constexpr Vector2 TopStartPosition{ 690.75f, 270.f };
		constexpr Vector2 BottomStartPosition{ 1229.25f, 810.f };
		constexpr float AnimationDuration = 1.f;
	}

	void RespawnWipeWidget::Play()
	{
		GM_ASSERT_RETURN(_topImage && _bottomImage, "Respawn Wipe Widget이 초기화되지 않았습니다.");

		_elapsedTime = 0.f;
		_isPlaying = true;
		ApplyAnimation(0.f);
		SetVisible(true);
	}

	void RespawnWipeWidget::Stop()
	{
		_elapsedTime = 0.f;
		_isPlaying = false;
		SetVisible(false);
	}

	std::unique_ptr<Widget> RespawnWipeWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		root->SetSizeRule(WidgetSizeRule::FillParent);

		_topImage = root->AddNamedChild<Image>(L"RespawnWipeTop", TopTextureKey);
		_bottomImage = root->AddNamedChild<Image>(L"RespawnWipeBottom", BottomTextureKey);
		ApplyAnimation(0.f);
		return root;
	}

	void RespawnWipeWidget::OnInitialize()
	{
		Stop();
	}

	void RespawnWipeWidget::OnTick(float deltaTime)
	{
		if (_isPlaying == false)
			return;

		_elapsedTime += std::max(0.f, deltaTime);
		const float ratio = std::clamp(_elapsedTime / AnimationDuration, 0.f, 1.f);
		ApplyAnimation(ratio);
		if (ratio >= 1.f)
			Stop();
	}

	void RespawnWipeWidget::ApplyAnimation(float ratio)
	{
		const float moveDistance = PanelSize.x * ratio;

		_topImage->SetGeometry({ TopStartPosition.x + moveDistance, TopStartPosition.y }, PanelSize);
		_bottomImage->SetGeometry({ BottomStartPosition.x - moveDistance, BottomStartPosition.y }, PanelSize);
	}
}
