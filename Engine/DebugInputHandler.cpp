#include "DebugInputHandler.h"
#include "Application.h"

namespace gm
{
	void DebugInputHandler::RegisterDebugType(const std::wstring& name, bool enabledByDefault)
	{
#if GM_ENABLE_DEBUG_TOOLS
		GM_ASSERT_RETURN(name.empty() == false, "DebugType 이름이 비어 있습니다.");

		if (_debugTypes.find(name) != _debugTypes.end())
			return;

		GM_ASSERT_RETURN(_nextBitIndex < 64, "DebugType 개수가 uint64 범위를 초과했습니다.");

		const DebugType type = 1ull << _nextBitIndex++;
		_debugTypes.emplace(name, type);

		if (enabledByDefault)
			Enable(name);
#endif
	}

	void DebugInputHandler::Enable(const std::wstring& name)
	{
#if GM_ENABLE_DEBUG_TOOLS
		_enabledTypes |= FindDebugType(name);
#endif
	}

	void DebugInputHandler::Disable(const std::wstring& name)
	{
#if GM_ENABLE_DEBUG_TOOLS
		_enabledTypes &= ~FindDebugType(name);
#endif
	}

	void DebugInputHandler::Toggle(const std::wstring& name)
	{
#if GM_ENABLE_DEBUG_TOOLS
		_enabledTypes ^= FindDebugType(name);
#endif
	}

	void DebugInputHandler::Clear()
	{
#if GM_ENABLE_DEBUG_TOOLS
		_enabledTypes = 0;
#endif
	}

	bool DebugInputHandler::IsTriggered(const std::wstring& name, KeyCode keycode, KeyState keyState)
	{
#if GM_ENABLE_DEBUG_TOOLS
		const DebugType debugType = FindDebugType(name);
		if ((_enabledTypes & debugType) == 0)
			return false;

		const Input& input = APPLICATION.GetInput();
		switch (keyState)
		{
		case KeyState::Down:
			return input.IsKeyDown(keycode);

		case KeyState::Repeat:
			return input.IsKeyRepeat(keycode);

		case KeyState::Up:
			return input.IsKeyUp(keycode);
		}
#endif

		return false;
	}

	DebugInputHandler::DebugType DebugInputHandler::FindDebugType(const std::wstring& name)
	{
#if GM_ENABLE_DEBUG_TOOLS
		const auto iter = _debugTypes.find(name);
		GM_ASSERT_RETURN_VAL(iter != _debugTypes.end(), 0, "%ls DebugType이 등록되지 않았습니다.", name.c_str());

		return iter->second;
#else
		(void)name;
		return 0;
#endif
	}
}
