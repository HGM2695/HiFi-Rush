#include "TriggeredLightColorComponent.h"

#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "LightComponent.h"

#include <utility>

namespace gm
{
	TriggeredLightColorComponent::TriggeredLightColorComponent(const BeatSystem& beatSystem, TriggeredLightColorDesc desc)
		: _beatSystem(beatSystem), _desc(std::move(desc))
	{}

	void TriggeredLightColorComponent::OnInitialize()
	{
		_light = GetOwner().GetComponent<LightComponent>();
		GM_ASSERT_RETURN(_light, "TriggeredLightColorComponent에 필요한 LightComponent가 없습니다.");
		_initialColor = _light->GetColor();

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "TriggeredLightColorComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset, [this](float startBeat) { ScheduleColorChange(startBeat); }, [this]() { ResetColor(); }), "TriggeredLightColorComponent의 Trigger Binding에 실패했습니다.");
		ResetColor();
	}

	void TriggeredLightColorComponent::OnTick(float)
	{
		if (_isApplied || _triggerBeat.has_value() == false || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < *_triggerBeat)
			return;
		ApplyColor();
	}

	void TriggeredLightColorComponent::ScheduleColorChange(float startBeat)
	{
		if (_isApplied || _triggerBeat.has_value())
			return;
		_triggerBeat = startBeat;
	}

	void TriggeredLightColorComponent::ResetColor()
	{
		_light->SetColor(_initialColor);
		_triggerBeat.reset();
		_isApplied = false;
	}

	void TriggeredLightColorComponent::ApplyColor()
	{
		_light->SetColor(_desc.color);
		_triggerBeat.reset();
		_isApplied = true;
	}
}
