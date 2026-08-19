#pragma once

#include "EngineCore.h"
#include "TextTypes.h"

namespace gm
{
	class IGraphicsDevice;

	class ITextRenderer
	{
	public:
		virtual ~ITextRenderer() = default;

		virtual bool Initialize(IGraphicsDevice& graphicsDevice) = 0;
		virtual bool RegisterFont(const std::wstring& fontKey, const std::wstring& fontFamilyName) = 0;
		virtual bool RegisterFontFile(const std::wstring& fontKey, const std::wstring& filePath) = 0;
		virtual void RequestDrawText(const std::wstring& text, const std::wstring& fontKey, const Vector2& position, float fontSize = 24.f, Color color = Colors::Black, 
			TextHorizontalAlignment horizontalAlignment = TextHorizontalAlignment::Left, TextVerticalAlignment verticalAlignment = TextVerticalAlignment::Top) = 0;
		virtual void Render() = 0;
		virtual void Clear() = 0;
	};
}
