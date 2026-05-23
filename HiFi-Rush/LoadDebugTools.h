#pragma once

#include "Application.h"
#include "UIManager.h"
#include "DebugTextWidget.h"

namespace gm
{
	inline void LoadUI();

	inline void LoadDebugTools()
	{
		LoadUI();
	}

	inline void LoadUI()
	{
		APPLICATION.GetUIManager().AddDebugUserWidget<DebugTextWidget>();
	}
}
