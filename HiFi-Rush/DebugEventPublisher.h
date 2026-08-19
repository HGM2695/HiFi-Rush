#pragma once

#if GM_ENABLE_DEBUG_TOOLS

namespace gm
{
	class DebugEventPublisher
	{
	public:
		void Initialize();
		void Tick();
	};
}

#endif
