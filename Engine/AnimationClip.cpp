#include "AnimationClip.h"
#include <algorithm>
#include <utility>

namespace gm
{
	const AnimationNotifyEvent* AnimationClip::FindNotify(const std::wstring& name) const
	{
		const auto iter = std::find_if(_notifyEvents.begin(), _notifyEvents.end(),
			[&name](const AnimationNotifyEvent& notifyEvent)
			{
				return notifyEvent.name == name;
			});
		return iter != _notifyEvents.end() ? &(*iter) : nullptr;
	}

	void AnimationClip::AddNotify(float time, const std::wstring& name)
	{
		GM_ASSERT_RETURN(time >= 0.f, "AnimationNotify의 time은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN(name.empty() == false, "문자열 기반 AnimationNotify는 이름이 필요합니다.");

		AnimationNotifyEvent notifyEvent{};
		notifyEvent.time = time;
		notifyEvent.name = name;
		_notifyEvents.push_back(std::move(notifyEvent));
		std::sort(_notifyEvents.begin(), _notifyEvents.end(),
			[](const AnimationNotifyEvent& lhs, const AnimationNotifyEvent& rhs)
			{
				return lhs.time < rhs.time;
			});
	}
}
