#include "DebugTextWidget.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "TextBlock.h"

namespace gm
{
	std::unique_ptr<Widget> DebugTextWidget::BuildWidgetTree()
	{
		auto textBlock = CreateRootWidget<TextBlock>();
		textBlock->SetPosition(Vector2{ 0.f, 0.f });

		return textBlock;
	}

	void DebugTextWidget::OnTick(float deltaTime)
	{
		_accTime += deltaTime;
		++_callCount;

		if (_accTime >= 1.f)
		{
			_fps = static_cast<float>(_callCount) / _accTime;
			_callCount = 0;
			_accTime = 0.f;
		}

		static_cast<TextBlock*>(GetRootWidget())->SetText(L"FPS : " + std::to_wstring(_fps));
	}
}

#endif
