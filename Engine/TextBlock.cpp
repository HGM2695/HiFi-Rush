#include "TextBlock.h"
#include "Application.h"
#include "ITextRenderer.h"

namespace gm
{
	void TextBlock::OnRender(const Vector2& absolutePosition)
	{
		if (_text.empty())
			return;

		APPLICATION.GetTextRenderer().RequestDrawText(_text, L"Engine.DefaultUI", absolutePosition, 24.f, _color);
	}
}
