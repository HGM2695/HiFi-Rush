#include "TriggerBinding.h"
#include "GMAssert.h"
#include "TriggerSystem.h"

#include <utility>

namespace gm
{
	bool TriggerBinding::Bind(TriggerSystem& triggerSystem, const std::wstring& triggerId, float beatOffset, ActivateHandler activateHandler, ResetHandler resetHandler)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(beatOffset >= 0.f, false, "Trigger Beat Offset은 0 이상이어야 합니다. triggerId=%ls", triggerId.c_str());
		GM_ASSERT_RETURN_VAL(activateHandler && resetHandler, false, "Trigger Handler가 유효하지 않습니다. triggerId=%ls", triggerId.c_str());

		_triggerId = triggerId;
		_beatOffset = beatOffset;
		_activateHandler = std::move(activateHandler);
		_resetHandler = std::move(resetHandler);
		triggerSystem.OnTrigger.Subscribe(_triggerConnection,
			[this](const TriggerEvent& event)
			{
				HandleTrigger(event);
			});
		return true;
	}

	void TriggerBinding::HandleTrigger(const TriggerEvent& event)
	{
		if (event.targetsAll == false && event.triggerId != _triggerId)
			return;

		if (event.type == TriggerType::Activate)
			_activateHandler(event.startBeat + _beatOffset);
		else
			_resetHandler();
	}
}
