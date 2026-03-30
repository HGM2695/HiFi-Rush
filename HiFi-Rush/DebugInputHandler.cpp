#include "DebugInputHandler.h"
#include "../Engine/Application.h"
#include "../Engine/Input.h"

namespace gm
{
	bool DebugInputHandler::isTriggered(const std::wstring& eventName)
	{
#ifdef _DEBUG
		const Input& input = APPLICATION.GetInput();

		bool result = false;
		if (_types & AudioTest)
			result = DispatchAudioTest(eventName, input);
		if (_types & PhysicsTest)
			result |= DispatchPhysicsTest(eventName, input);

		return result;
#else
		return false;
#endif
	}

	bool DebugInputHandler::DispatchAudioTest(const std::wstring& eventName, const Input& input)
	{
#ifdef _DEBUG
		if (eventName == L"Play" && input.IsKeyDown(KeyCode::P))
			return true;
		if (eventName == L"Stop" && input.IsKeyDown(KeyCode::S))
			return true;
		if (eventName == L"Pause" && input.IsKeyDown(KeyCode::D0))
			return true;
		if (eventName == L"Resume" && input.IsKeyDown(KeyCode::D1))
			return true;
		if (eventName == L"StopAll" && input.IsKeyDown(KeyCode::D2))
			return true;


		return false;
#else
		return false;
#endif
	}

	bool DebugInputHandler::DispatchPhysicsTest(const std::wstring& eventName, const Input& input)
	{
#ifdef _DEBUG

		return false;
#else
		return false;
#endif
	}
}
