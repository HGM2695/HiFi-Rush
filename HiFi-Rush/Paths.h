#pragma once

#include <string>

namespace gm
{
	inline std::wstring CombinePath(const std::wstring& root, const std::wstring& relativePath)
	{
		std::wstring path(root);

		if (path.empty() == false && path.back() != L'/')
			path += L'/';

		if (relativePath.empty())
			return path;

		if (relativePath.front() == L'/' || relativePath.front() == L'\\')
			path += relativePath.substr(1);
		else
			path += relativePath;

		return path;
	}

	inline std::wstring GetResourcePath(const std::wstring& relativePath)
	{
		return CombinePath(L"Resources", relativePath);
	}

	inline std::wstring GetAudioPath(const std::wstring& relativePath)
	{
		return CombinePath(L"Resources/Audio", relativePath);
	}

	inline std::wstring GetTexturePath(const std::wstring& relativePath)
	{
		return CombinePath(L"Resources/Texture", relativePath);
	}

	inline std::wstring GetAnimationPath(const std::wstring& relativePath)
	{
		return CombinePath(L"Resources/Animation", relativePath);
	}
}
