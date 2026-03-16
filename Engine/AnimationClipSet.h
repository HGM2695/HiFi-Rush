#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace gm
{
	class AnimationClip;

	class AnimationClipSet
	{
	public:
		template <typename TClip>
		std::shared_ptr<TClip> FindClip(const std::wstring& name) const
		{
			auto clip = FindClipBase(name);
			if (clip == nullptr)
				return nullptr;

			return std::static_pointer_cast<TClip>(clip);
		}

		bool							AddClip(const std::wstring& name, const std::shared_ptr<AnimationClip>& clip);
		bool							HasClip(const std::wstring& name) const;
		void							Clear();

	private:
		std::shared_ptr<AnimationClip>	FindClipBase(const std::wstring& name) const;

	private:
		std::unordered_map<std::wstring, std::shared_ptr<AnimationClip>> _clipSet;
	};
}