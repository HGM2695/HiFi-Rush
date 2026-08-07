#pragma once

#include "EngineCore.h"
#include "TextTypes.h"

namespace gm
{
	struct CameraViewInfo;
	class IGraphicsDevice;

	class IDebugRenderer
	{
	public:
		virtual ~IDebugRenderer() = default;

		void SetEnabled(bool isEnabled)
		{
			_isEnabled = isEnabled;
			if (_isEnabled == false)
				Clear();
		}
		bool IsEnabled() const { return _isEnabled; }

		virtual bool Initialize(IGraphicsDevice& graphicsDevice) = 0;
		virtual void Render(const CameraViewInfo& viewInfo) = 0;

		virtual void RequestDrawLine(const Vector2& start, const Vector2& end, Color color = Colors::Green) = 0;
		virtual void RequestDrawLine(const Vector3& start, const Vector3& end, Color color = Colors::Green) = 0;
		virtual void RequestDrawRect(const Vector2& center, const Vector2& size, Color color = Colors::Green) = 0;
		virtual void RequestDrawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, Color color = Colors::Green) = 0;
		virtual void RequestDrawQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, Color color = Colors::Green) = 0;
		virtual void RequestDrawCircle(const Vector2& center, float radius, Color color = Colors::Green, uint32 segments = 48) = 0;
		virtual void RequestDrawCircle(const Vector3& center, float radius, Color color = Colors::Green, uint32 segments = 48) = 0;
		virtual void RequestDrawBox(const Vector3& center, const Vector3& extent, Color color = Colors::Green) = 0;
		virtual void RequestDrawBox(const BoundingBox& box, Color color = Colors::Green) = 0;
		virtual void RequestDrawOBB(const Vector3& center, const Vector3& extents, const Quaternion& quaternion, Color color = Colors::Green) = 0;
		virtual void RequestDrawOBB(const BoundingOrientedBox& obb, Color color = Colors::Green) = 0;
		virtual void RequestDrawSphere(const Vector3& center, float radius, Color color = Colors::Green, uint32 segments = 32) = 0;
		virtual void RequestDrawSphere(const BoundingSphere& sphere, Color color = Colors::Green) = 0;
		virtual void RequestDrawRay(const Vector3& origin, const Vector3& direction, float length, Color color = Colors::Green) = 0;
		virtual void RequestDrawRay(const Ray& ray, float length, Color color = Colors::Green) = 0;
		virtual void RequestDrawText(const std::wstring& content, const Vector2& viewPosition, float fontSize = 14.f, Color color = Colors::Black,
			TextHorizontalAlignment horizontalAlignment = TextHorizontalAlignment::Left, TextVerticalAlignment verticalAlignment = TextVerticalAlignment::Top) = 0;

		virtual void Clear() = 0;

	private:
		bool _isEnabled = true;
	};
}
