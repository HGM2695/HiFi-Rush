#include "AnimationClip.h"
#include <algorithm>

namespace gm
{
	void AnimationClip::AddNotify(float time, const std::wstring& name)
	{
		GM_ASSERT_RETURN(time >= 0.f, "AnimationNotify의 time은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN(name.empty() == false, "문자열 기반 AnimationNotify는 이름이 필요합니다.");

		_notifyEvents.push_back({ time, name });
		std::sort(_notifyEvents.begin(), _notifyEvents.end(),
			[](const AnimationNotifyEvent& lhs, const AnimationNotifyEvent& rhs)
			{
				return lhs.time < rhs.time;
			});
	}
}
