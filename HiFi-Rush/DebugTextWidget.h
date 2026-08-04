#pragma once

#include "UserWidget.h"

#if GM_ENABLE_DEBUG_TOOLS

namespace gm
{
	class BeatSystem;

	class DebugTextWidget : public UserWidget
	{
	public:
		explicit DebugTextWidget(const BeatSystem& beatSystem) : _beatSystem(beatSystem) {}

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;

		virtual void			OnTick(float deltaTime) override;

	private:
		const BeatSystem&	_beatSystem;
		uint32				_callCount = 0;
		float				_fps = 0.f;
		float				_accTime = 0.f;
		float				_beatPulseTime = 0.f;
	};
}

#endif
