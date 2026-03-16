#include "AnimationClipSet.h"
#include "AnimationClip.h"
#include "GMAssert.h"

namespace gm
{
	bool AnimationClipSet::AddClip(const std::wstring& name, const std::shared_ptr<AnimationClip>& clip)
	{
		GM_ASSERT_RETURN_VAL(name.empty() == false, false, "클립 이름이 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(clip, false, "SpriteAnimationClip이 nullptr입니다.");
		GM_ASSERT_RETURN_VAL(clip->GetFrameCount() > 0, false, "SpriteAnimationClip에 프레임이 없습니다.");

		_clipSet[name] = clip;
		return true;
	}

	bool AnimationClipSet::HasClip(const std::wstring& name) const
	{
		return _clipSet.find(name) != _clipSet.end();
	}

	void AnimationClipSet::Clear()
	{
		_clipSet.clear();
	}

	std::shared_ptr<AnimationClip> AnimationClipSet::FindClipBase(const std::wstring& name) const
	{
		auto iter = _clipSet.find(name);
		if (iter == _clipSet.end())
			return nullptr;

		return iter->second;
	}
}
