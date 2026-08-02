#pragma once

#include "EngineCore.h"
#include "Input.h"

#include <string>
#include <unordered_map>

namespace gm
{
	class Input;

	class DebugInputHandler
	{
	public:
		static void RegisterDebugType(const std::wstring& name, bool enabledByDefault = false);

		static void Enable(const std::wstring& name);
		static void Disable(const std::wstring& name);
		static void Toggle(const std::wstring& name);
		static void Clear();

		static bool IsTriggered(const std::wstring& name, KeyCode keycode, KeyState keyState = KeyState::Down);

	private:
		using DebugType = uint64;
		static DebugType FindDebugType(const std::wstring& name);

	private:
#if GM_ENABLE_DEBUG_TOOLS
		static inline uint64 _enabledTypes = 0;
		static inline uint32 _nextBitIndex = 0;
		static inline std::unordered_map<std::wstring, DebugType> _debugTypes{};
#endif
	};
}
