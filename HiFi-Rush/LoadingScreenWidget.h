#pragma once

#include "UserWidget.h"

namespace gm
{
	class LoadingScreenWidget : public UserWidget
	{
	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
	};
}
