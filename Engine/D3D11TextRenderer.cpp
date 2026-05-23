#include "D3D11TextRenderer.h"
#include "D3D11GraphicsDevice.h"
#include "BuiltinGraphicsResources.h"
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dxgi.h>

namespace gm
{
	namespace
	{
		float CalculateTextLeft(float anchorX, float textWidth, TextHorizontalAlignment alignment)
		{
			switch (alignment)
			{
			case TextHorizontalAlignment::Left:
				return anchorX;
			case TextHorizontalAlignment::Center:
				return anchorX - 0.5f * textWidth;
			case TextHorizontalAlignment::Right:
				return anchorX - 1.f * textWidth;
			default:
				GM_ASSERT_RETURN_VAL(false, 0.f, "지원하지 않는 가로 텍스트 정렬입니다.");
			}
		}

		float CalculateTextTop(float anchorY, float textHeight, TextVerticalAlignment alignment)
		{
			switch (alignment)
			{
			case TextVerticalAlignment::Top:
				return anchorY;
			case TextVerticalAlignment::Center:
				return anchorY - 0.5f * textHeight;
			case TextVerticalAlignment::Bottom:
				return anchorY - 1.f * textHeight;
			default:
				GM_ASSERT_RETURN_VAL(false, 0.f, "지원하지 않는 세로 텍스트 정렬입니다.");
			}
		}
	}

	D3D11TextRenderer::D3D11TextRenderer() = default;
	D3D11TextRenderer::~D3D11TextRenderer() = default;

	bool D3D11TextRenderer::Initialize(IGraphicsDevice& graphicsDevice)
	{
		GM_ASSERT_RETURN_VAL(CreateDeviceResources(graphicsDevice), false, "D3D11TextRenderer 리소스 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterFont(BuiltinResourceKey::DefaultUIFont, L"Segoe UI"), false, "D3D11TextRenderer TextFormat 생성에 실패했습니다.");

		return true;
	}

	bool D3D11TextRenderer::RegisterFont(const std::wstring& fontKey, const std::wstring& fontFamilyName)
	{
		GM_ASSERT_RETURN_VAL(_fontFamilyNameMapper.find(fontKey) == _fontFamilyNameMapper.end(), false, "%ls Key는 이미 존재합니다.", fontKey.c_str());
		_fontFamilyNameMapper[fontKey] = fontFamilyName;
		
		return true;
	}

	void D3D11TextRenderer::RequestDrawText(const std::wstring& text, const std::wstring& fontKey, const Vector2& position, float fontSize, Color color, TextHorizontalAlignment horizontalAlignment, TextVerticalAlignment verticalAlignment)
	{
		GM_ASSERT_RETURN(_fontFamilyNameMapper.find(fontKey) != _fontFamilyNameMapper.end(), "해당 fontKey는 등록되지 않았습니다.");

		const std::wstring& familyName = _fontFamilyNameMapper[fontKey];
		TextFormatKey textFormatKey = ToTextFormatKey(familyName, fontSize);

		if (_textFormatCache.find(textFormatKey) == _textFormatCache.end())
			GM_ASSERT_RETURN(CreateTextFormat(familyName, fontSize), "TextFormat 생성 실패 %ls", familyName.c_str());

		_drawList.push_back(DrawItem{ _textFormatCache[textFormatKey], text, position, color, horizontalAlignment, verticalAlignment });
	}

	void D3D11TextRenderer::Render()
	{
		if (_drawList.empty())
			return;

		_d2dContext->BeginDraw();
		_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

		for (const DrawItem& item : _drawList)
		{	
			_brush->SetColor(D2D1::ColorF(item.color.x, item.color.y, item.color.z, item.color.w));

			Rect rect = CalcDrawRect(item);
			const D2D1_RECT_F layoutRect = D2D1::RectF(rect.left, rect.top, rect.Right(), rect.Bottom());

			_d2dContext->DrawTextW(
				item.text.c_str(),
				static_cast<UINT32>(item.text.length()),
				item.textFormat.Get(),
				layoutRect,
				_brush.Get()
			);
		}

		const HRESULT hr = _d2dContext->EndDraw();
		GM_ASSERT(SUCCEEDED(hr), "D3D11TextRenderer DrawText에 실패했습니다.");

		Clear();
	}

	void D3D11TextRenderer::Clear()
	{
		_drawList.clear();
	}

	bool D3D11TextRenderer::CreateDeviceResources(IGraphicsDevice& graphicsDevice)
	{
		D3D11GraphicsDevice& d3d11Device = static_cast<D3D11GraphicsDevice&>(graphicsDevice);

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		HRESULT hr = d3d11Device.GetNativeDevice()->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "IDXGIDevice QueryInterface에 실패했습니다.");

		D2D1_FACTORY_OPTIONS factoryOptions{};
#ifdef _DEBUG
		factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

		hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory1),
			&factoryOptions,
			reinterpret_cast<void**>(_d2dFactory.GetAddressOf())
		);
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D Factory 생성에 실패했습니다.");

		hr = _d2dFactory->CreateDevice(dxgiDevice.Get(), _d2dDevice.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D Device 생성에 실패했습니다.");

		hr = _d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, _d2dContext.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D DeviceContext 생성에 실패했습니다.");

		Microsoft::WRL::ComPtr<IDXGISurface> backBufferSurface;
		hr = d3d11Device.GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(backBufferSurface.GetAddressOf()));
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D 렌더 타겟용 BackBuffer Surface를 가져오지 못했습니다.");

		const D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);

		hr = _d2dContext->CreateBitmapFromDxgiSurface(backBufferSurface.Get(), &bitmapProperties, _renderTarget.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D BackBuffer Bitmap 생성에 실패했습니다.");

		_d2dContext->SetTarget(_renderTarget.Get());

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(_dwriteFactory.GetAddressOf()));
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "DirectWrite Factory 생성에 실패했습니다.");

		hr = _d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), _brush.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "D2D Text Brush 생성에 실패했습니다.");

		return true;
	}

	Rect D3D11TextRenderer::CalcDrawRect(const DrawItem& item)
	{
		Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
		_dwriteFactory->CreateTextLayout(
			item.text.c_str(),
			static_cast<UINT32>(item.text.length()),
			item.textFormat.Get(),
			4096.f,
			1024.f,
			layout.GetAddressOf()
		);

		DWRITE_TEXT_METRICS metrics{};
		layout->GetMetrics(&metrics);

		float left = CalculateTextLeft(item.position.x, metrics.width, item.horizontalAlignment);
		float top = CalculateTextTop(item.position.y, metrics.height, item.verticalAlignment);

		return Rect{ left, top, metrics.width, metrics.height };
	}

	D3D11TextRenderer::TextFormatKey D3D11TextRenderer::ToTextFormatKey(const std::wstring& fontFamilyName, float fontSize)
	{
		return TextFormatKey(fontFamilyName, static_cast<uint32>(std::round(fontSize * 100.f)));
	}

	bool D3D11TextRenderer::CreateTextFormat(const std::wstring& fontFamilyName, float fontSize)
	{
		Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;

		const HRESULT hr = _dwriteFactory->CreateTextFormat(
			fontFamilyName.c_str(),
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"ko-kr",
			textFormat.GetAddressOf()
		);

		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "[%ls] 생성에 실패했습니다.", fontFamilyName.c_str());

		textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		_textFormatCache.insert({ ToTextFormatKey(fontFamilyName, fontSize), textFormat });

		return true;
	}
}
