#pragma once

#include "ITextRenderer.h"
#include "Rect.h"
#include <vector>
#include <wrl/client.h>

struct ID2D1Bitmap1;
struct ID2D1Device;
struct ID2D1DeviceContext;
struct ID2D1Factory1;
struct ID2D1SolidColorBrush;
struct IDWriteFactory;
struct IDWriteTextFormat;

namespace gm
{
	class D3D11TextRenderer final : public ITextRenderer
	{
	public:
		D3D11TextRenderer();
		virtual ~D3D11TextRenderer();

		virtual bool Initialize(IGraphicsDevice& graphicsDevice) override;
		virtual bool RegisterFont(const std::wstring& fontKey, const std::wstring& fontFamilyName) override;
		virtual void RequestDrawText(const std::wstring& text, const std::wstring& fontKey, const Vector2& position, float fontSize, Color color,
			TextHorizontalAlignment horizontalAlignment, TextVerticalAlignment verticalAlignment) override;
		virtual void Render() override;
		virtual void Clear() override;

	private:
		struct TextFormatKey
		{
			std::wstring	fontFamilyName;
			uint32			scaledFontSize;

			bool operator==(const TextFormatKey& rhs) const
			{
				return fontFamilyName == rhs.fontFamilyName && scaledFontSize == rhs.scaledFontSize;
			}
		};

		struct TextFormatKeyHasher
		{
			size_t operator()(const TextFormatKey& key) const
			{
				const std::size_t h1 = std::hash<std::wstring>{}(key.fontFamilyName);
				const std::size_t h2 = std::hash<uint32>{}(key.scaledFontSize);

				return h1 ^ (h2 << 1);
			}
		};

		struct DrawItem
		{
			Microsoft::WRL::ComPtr<IDWriteTextFormat>	textFormat;
			std::wstring								text;
			Vector2										position;
			Color										color = Colors::White;
			TextHorizontalAlignment						horizontalAlignment;
			TextVerticalAlignment						verticalAlignment;
		};

		bool CreateTextFormat(const std::wstring& fontFamilyName, float fontSize);
		TextFormatKey ToTextFormatKey(const std::wstring& fontFamilyName, float fontSize);
		bool CreateDeviceResources(IGraphicsDevice& graphicsDevice);
		Rect CalcDrawRect(const DrawItem& item);

	private:
		std::unordered_map<TextFormatKey, Microsoft::WRL::ComPtr<IDWriteTextFormat>, TextFormatKeyHasher>	_textFormatCache;
		std::unordered_map<std::wstring, std::wstring>														_fontFamilyNameMapper;

		Microsoft::WRL::ComPtr<ID2D1Factory1>			_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device>				_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext>		_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>			_renderTarget;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_brush;
		Microsoft::WRL::ComPtr<IDWriteFactory>			_dwriteFactory;
		std::vector<DrawItem>							_drawList;
	};
}
