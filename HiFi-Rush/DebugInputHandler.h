#pragma once

#include <cstdint>
#include <string>
#include "../Engine/Input.h"

namespace gm
{
	class Input;

	enum DebugType : uint32_t
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
#ifdef _DEBUG
			_types |= type;
#else
			(void)type;
#endif
		}

		static inline void Disable(DebugType type)
		{
#ifdef _DEBUG
			_types &= ~type;
#else
			(void)type;
#endif
		}

		static inline void Toggle(DebugType type)
		{
#ifdef _DEBUG
			_types ^= type;
#else
			(void)type;
#endif
		}

		static inline void Clear()
		{
#ifdef _DEBUG
			_types = 0;
#endif
		}

		static bool IsTriggered(DebugType debugType, KeyCode keycode, KeyState keyState = KeyState::Down);

	private:
#ifdef _DEBUG
		static inline uint32_t _types = AudioTest | PhysicsTest;
#endif
	};
}
