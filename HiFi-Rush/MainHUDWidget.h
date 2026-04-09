#pragma once

#include "UserWidget.h"

namespace gm
{
	class MainHUDWidget : public UserWidget
	{
	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
	};
}
