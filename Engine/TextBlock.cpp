#include "TextBlock.h"
#include "Application.h"
#include "ITextRenderer.h"
#include "BuiltinGraphicsResources.h"

namespace gm
{
	TextBlock::TextBlock()
	{
		SetName(L"TextBlock");
	}

	void TextBlock::OnRender(const WidgetGeometry& geometry)
	{
		if (_text.empty())
			return;

		APPLICATION.GetTextRenderer().RequestDrawText(_text, BuiltinResourceKey::DefaultUIFont, geometry.center, _size, _color, _horizonAlign, _verticalAlign);
	}
}
