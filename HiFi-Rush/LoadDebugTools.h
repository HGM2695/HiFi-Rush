#pragma once

#include "Application.h"
#include "UIManager.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugTextWidget.h"
#endif

namespace gm
{
	inline void LoadDebugTools()
	{
#if GM_ENABLE_DEBUG_TOOLS
		APPLICATION.GetUIManager().AddDebugUserWidget<DebugTextWidget>();
#endif
	}
}
