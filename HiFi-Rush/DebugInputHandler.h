#pragma once

#include "EngineCore.h"
#include "Input.h"

namespace gm
{
	class Input;

	enum DebugType : uint32
	{
		None = 0u,
		AudioTest = 1u << 0,
		PhysicsTest = 1u << 1,
	};

	class DebugInputHandler
	{
	public:
		static inline void Enable(DebugType type)
		{
#if GM_ENABLE_DEBUG_TOOLS
			_types |= type;
#else
			(void)type;
#endif
		}

		static inline void Disable(DebugType type)
		{
#if GM_ENABLE_DEBUG_TOOLS
			_types &= ~type;
#else
			(void)type;
#endif
		}

		static inline void Toggle(DebugType type)
		{
#if GM_ENABLE_DEBUG_TOOLS
			_types ^= type;
#else
			(void)type;
#endif
		}

		static inline void Clear()
		{
#if GM_ENABLE_DEBUG_TOOLS
			_types = 0;
#endif
		}

		static bool IsTriggered(DebugType debugType, KeyCode keycode, KeyState keyState = KeyState::Down);

	private:
#if GM_ENABLE_DEBUG_TOOLS
		static inline uint32 _types = AudioTest | PhysicsTest;
#endif
	};
}
