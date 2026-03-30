#pragma once

#include <cstdint>
#include <string>

namespace gm
{
	class Input;

	enum DebugType : uint32_t
	{
		None = 0u,
		AudioTest = 1u << 0,
		PhysicsTest = 1u << 1,

		Count
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

		static bool isTriggered(const std::wstring& eventName);

	private:
		static bool DispatchAudioTest(const std::wstring& eventName, const Input& input);
		static bool DispatchPhysicsTest(const std::wstring& eventName, const Input& input);

	private:
#ifdef _DEBUG
		static inline uint32_t _types = AudioTest | PhysicsTest;
#endif
	};
}
