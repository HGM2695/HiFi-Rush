#pragma once

#include "Event.h"

#include <functional>
#include <string>

namespace gm
{
	class TriggerSystem;
	struct TriggerEvent;

	class TriggerBinding
	{
	public:
		using ActivateHandler = std::function<void(float)>;
		using ResetHandler = std::function<void()>;

		bool Bind(TriggerSystem& triggerSystem, const std::wstring& triggerId, float beatOffset, ActivateHandler activateHandler, ResetHandler resetHandler);

	private:
		void HandleTrigger(const TriggerEvent& event);

	private:
		std::wstring	_triggerId{};
		float			_beatOffset = 0.f;
		ActivateHandler	_activateHandler{};
		ResetHandler	_resetHandler{};
		EventConnection	_triggerConnection{};
	};
}
