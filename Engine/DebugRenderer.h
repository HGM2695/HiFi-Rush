#pragma once

struct HDC__;
typedef struct HDC__* HDC;

namespace gm::debug
{
	class DebugRenderer
	{
	public:
		explicit DebugRenderer(HDC hDC);
		~DebugRenderer();

	public:
		void DrawLine(int x1, int y1, int x2, int y2);
		void DrawRect(int left, int top, int right, int bottom);
		void DrawCircle(int centerX, int centerY, int radius);

	private:
		HDC		_hDC = nullptr;
	};
}