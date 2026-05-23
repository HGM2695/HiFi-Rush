#include "Border.h"
#include "Application.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Rect.h"
#include <algorithm>

namespace gm
{
	Border::Border()
	{
		SetName(L"Border");
	}

	void Border::OnRender(const WidgetGeometry& geometry)
	{
		if (geometry.size.x <= 0.f || geometry.size.y <= 0.f)
			return;

		SubmitBackGround(geometry);
		SubmitOutline(geometry);
	}

	void Border::SubmitBackGround(const WidgetGeometry& geometry)
	{
		ColorQuadRenderItem item{};
		item.screenCenter = geometry.center;
		item.size = geometry.size;
		item.color = _backgroundColor;

		APPLICATION.GetRenderer().SubmitColorQuad(item);
	}

	void Border::SubmitOutline(const WidgetGeometry& geometry)
	{
		if (_borderThickness <= 0.f)
			return;

		Rect rect = Rect::FromCenterSize(geometry.center, geometry.size);
		const float horizontalLength = std::max(0.f, geometry.size.x - _borderThickness * 2.f);

		// left
		APPLICATION.GetRenderer().SubmitColorQuad(ColorQuadRenderItem
			{
				Vector2{rect.left + 0.5f * _borderThickness, geometry.center.y },
				Vector2{_borderThickness, geometry.size.y},
				_outlineColor
			});

		// right
		APPLICATION.GetRenderer().SubmitColorQuad(ColorQuadRenderItem
			{
				Vector2{rect.Right() - 0.5f * _borderThickness, geometry.center.y},
				Vector2{_borderThickness, geometry.size.y},
				_outlineColor
			});

		// top
		APPLICATION.GetRenderer().SubmitColorQuad(ColorQuadRenderItem
			{
				Vector2{geometry.center.x, rect.top + 0.5f * _borderThickness },
				Vector2{horizontalLength, _borderThickness},
				_outlineColor
			});

		// bottom
		APPLICATION.GetRenderer().SubmitColorQuad(ColorQuadRenderItem
			{
				Vector2{geometry.center.x, rect.Bottom() - 0.5f * _borderThickness},
				Vector2{horizontalLength, _borderThickness},
				_outlineColor
			});
	}
}
