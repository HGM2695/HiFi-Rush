#pragma once

#include "IDebugRenderer.h"
#include <vector>

namespace gm
{
	class D3D11TextRenderer;

	class D3D11DebugRenderer final : public IDebugRenderer
	{

	public:
		D3D11DebugRenderer(D3D11TextRenderer* textRenderer);
		virtual ~D3D11DebugRenderer();

		virtual bool Initialize(IGraphicsDevice& graphicsDevice) override;
		virtual void Render(const CameraViewInfo& viewInfo) override;

		virtual void RequestDrawLine(const Vector2& start, const Vector2& end, Color color = Colors::Green) override;
		virtual void RequestDrawLine(const Vector3& start, const Vector3& end, Color color = Colors::Green) override;
		virtual void RequestDrawRect(const Vector2& center, const Vector2& size, Color color = Colors::Green) override;
		virtual void RequestDrawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, Color color = Colors::Green) override;
		virtual void RequestDrawQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, Color color = Colors::Green) override;
		virtual void RequestDrawCircle(const Vector2& center, float radius, Color color = Colors::Green, uint32 segments = 48) override;
		virtual void RequestDrawCircle(const Vector3& center, float radius, Color color = Colors::Green, uint32 segments = 48) override;
		virtual void RequestDrawBox(const Vector3& center, const Vector3& extent, Color color = Colors::Green) override;
		virtual void RequestDrawBox(const BoundingBox& box, Color color = Colors::Green) override;
		virtual void RequestDrawOBB(const Vector3& center, const Vector3& extents, const Quaternion& quaternion, Color color = Colors::Green) override;
		virtual void RequestDrawOBB(const BoundingOrientedBox& obb, Color color = Colors::Green) override;
		virtual void RequestDrawSphere(const Vector3& center, float radius, Color color = Colors::Green, uint32 segments = 32) override;
		virtual void RequestDrawSphere(const BoundingSphere& sphere, Color color = Colors::Green) override;
		virtual void RequestDrawRay(const Vector3& origin, const Vector3& direction, float length, Color color = Colors::Green) override;
		virtual void RequestDrawRay(const Ray& ray, float length, Color color = Colors::Green) override;
		virtual void RequestDrawText(const std::wstring& content, const Vector2& viewPosition, float fontSize = 14.f, Color color = Colors::Black) override;

		virtual void Clear() override;

	private:
		struct DebugLine
		{
			Vector3 start;
			Vector3 end;
			Color	color;
		};

		struct DebugTriangle
		{
			Vector3 a;
			Vector3 b;
			Vector3 c;
			Color	color;
		};

		struct DebugQuad
		{
			Vector3 a;
			Vector3 b;
			Vector3 c;
			Vector3 d;
			Color	color;
		};

		struct DebugRing
		{
			Vector3 center;
			Vector3 majorAxis;
			Vector3 minorAxis;
			Color	color;
		};

		struct DebugBox
		{
			BoundingBox box;
			Color	color;
		};

		struct DebugOrientedBox
		{
			BoundingOrientedBox obb;
			Color				color;
		};

		struct DebugSphere
		{
			BoundingSphere sphere;
			Color	color;
		};

		struct DebugRay
		{
			Ray		ray;
			float	length = 0.f;
			Color	color;
		};

		struct DirectXTKResources;

	private:
		D3D11TextRenderer*					_textRenderer;

		std::unique_ptr<DirectXTKResources> _dxResources;
		std::vector<DebugLine>				_lines;
		std::vector<DebugTriangle>			_triangles;
		std::vector<DebugQuad>				_quads;
		std::vector<DebugRing>				_rings;
		std::vector<DebugBox>				_boxes;
		std::vector<DebugOrientedBox>		_obbs;
		std::vector<DebugSphere>			_spheres;
		std::vector<DebugRay>				_rays;
	};
}
