#include "DebugRenderer.h"
#include "GMAssert.h"
#include <windows.h>

namespace gm::debug
{
	DebugRenderer::DebugRenderer(HDC hDC)
		: _hDC(hDC)
	{
#ifdef _DEBUG
		GM_ASSERT_RETURN(_hDC, "DC가 nullptr입니다.");
#endif
	}

	DebugRenderer::~DebugRenderer() = default;

	void DebugRenderer::DrawLine(int x1, int y1, int x2, int y2)
	{
#ifdef _DEBUG
		MoveToEx(_hDC, x1, y1, nullptr);
		LineTo(_hDC, x2, y2);
#endif
	}

	void DebugRenderer::DrawRect(int left, int top, int right, int bottom)
	{
#ifdef _DEBUG
		Rectangle(_hDC, left, top, right, bottom);
#endif
	}

	void DebugRenderer::DrawCircle(int centerX, int centerY, int radius)
	{
#ifdef _DEBUG
		Ellipse(
			_hDC,
			centerX - radius,
			centerY - radius,
			centerX + radius,
			centerY + radius
		);
#endif
	}
}
