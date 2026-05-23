#pragma once

#include "UserWidget.h"

namespace gm
{
	class DebugTextWidget : public UserWidget
	{
	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;

		virtual void			OnTick(float deltaTime) override;

	private:
		uint32	_callCount = 0;
		float	_fps = 0.f;
		float	_accTime = 0.f;
	};
}

