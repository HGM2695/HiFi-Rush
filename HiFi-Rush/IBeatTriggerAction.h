#pragma once

namespace gm
{
	class IBeatTriggerAction
	{
	public:
		virtual ~IBeatTriggerAction() = default;

		virtual void Schedule(float startBeat) = 0;
		virtual void Reset() = 0;
	};
}
