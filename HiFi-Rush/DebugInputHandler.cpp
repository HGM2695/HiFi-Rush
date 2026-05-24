#include "DebugInputHandler.h"
#include "Application.h"

namespace gm
{
	bool DebugInputHandler::IsTriggered(DebugType debugType, KeyCode keycode, KeyState keyState)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if ((_types & debugType) == DebugType::None)
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
}
