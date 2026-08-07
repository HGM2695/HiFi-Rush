#include "D3D11DebugRenderer.h"
#include "CameraViewInfo.h"
#include "D3D11GraphicsDevice.h"
#include "DebugDraw.h"
#include "D3D11TextRenderer.h"
#include "BuiltinGraphicsResources.h"

#include <d3d11.h>
#include <directxtk/DirectXHelpers.h>
#include <directxtk/Effects.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/VertexTypes.h>
#include <wrl/client.h>

namespace gm
{
	struct D3D11DebugRenderer::DirectXTKResources
	{
		std::unique_ptr<DirectX::BasicEffect> effect;
		std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> primitiveBatch;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		ID3D11DeviceContext* context = nullptr;
	};

	D3D11DebugRenderer::D3D11DebugRenderer(D3D11TextRenderer* textRenderer) : _textRenderer(textRenderer) {}
	D3D11DebugRenderer::~D3D11DebugRenderer() = default;

	bool D3D11DebugRenderer::Initialize(IGraphicsDevice& graphicsDevice)
	{
#if GM_ENABLE_DEBUG_TOOLS
		D3D11GraphicsDevice& d3d11Device = static_cast<D3D11GraphicsDevice&>(graphicsDevice);
		ID3D11Device* nativeDevice = d3d11Device.GetNativeDevice();
		ID3D11DeviceContext* nativeContext = d3d11Device.GetImmediateContext();

		GM_ASSERT_RETURN_VAL(nativeDevice, false, "D3D11 Device가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(nativeContext, false, "D3D11 DeviceContext가 유효하지 않습니다.");

		_dxResources = std::make_unique<DirectXTKResources>();
		_dxResources->context = nativeContext;
		_dxResources->effect = std::make_unique<DirectX::BasicEffect>(nativeDevice);
		_dxResources->effect->SetVertexColorEnabled(true);
		_dxResources->effect->SetLightingEnabled(false);
		_dxResources->effect->SetTextureEnabled(false);

		const HRESULT hr = DirectX::CreateInputLayoutFromEffect<DirectX::VertexPositionColor>(nativeDevice, _dxResources->effect.get(), _dxResources->inputLayout.GetAddressOf());
		GM_ASSERT_RETURN_VAL(SUCCEEDED(hr), false, "DebugRenderer InputLayout 생성 실패 hr=0x%08X", static_cast<unsigned int>(hr));

		_dxResources->primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(nativeContext);
#endif
		return true;
	}

	void D3D11DebugRenderer::Render(const CameraViewInfo& viewInfo)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
		{
			Clear();
			return;
		}

		if (_lines.empty() && _triangles.empty() && _quads.empty() && _rings.empty() && _boxes.empty() && _obbs.empty() && _spheres.empty() && _rays.empty())
			return;

		GM_ASSERT_RETURN(_dxResources && _dxResources->effect && _dxResources->primitiveBatch && _dxResources->context, "DebugRenderer가 초기화되지 않았습니다.");

		const Matrix world = DirectX::XMMatrixIdentity();
		const Matrix view = DirectX::XMLoadFloat4x4(&viewInfo.view);
		const Matrix projection = DirectX::XMLoadFloat4x4(&viewInfo.projection);

		_dxResources->effect->SetMatrices(world, view, projection);
		_dxResources->effect->Apply(_dxResources->context);
		_dxResources->context->IASetInputLayout(_dxResources->inputLayout.Get());

		_dxResources->primitiveBatch->Begin();

		for (const DebugTriangle& triangle : _triangles)
			debug_draw::DrawTriangle(_dxResources->primitiveBatch.get(), triangle.a, triangle.b, triangle.c, triangle.color);

		for (const DebugQuad& quad : _quads)
			debug_draw::DrawQuad(_dxResources->primitiveBatch.get(), quad.a, quad.b, quad.c, quad.d, quad.color);

		for (const DebugRing& ring : _rings)
			debug_draw::DrawRing(_dxResources->primitiveBatch.get(), ring.center, ring.majorAxis, ring.minorAxis, ring.color);

		for (const DebugBox& box : _boxes)
			debug_draw::Draw(_dxResources->primitiveBatch.get(), box.box, box.color);

		for (const DebugOrientedBox& obb : _obbs)
			debug_draw::Draw(_dxResources->primitiveBatch.get(), obb.obb, obb.color);

		for (const DebugSphere& sphere : _spheres)
			debug_draw::Draw(_dxResources->primitiveBatch.get(), sphere.sphere, sphere.color);

		for (const DebugLine& line : _lines)
		{
			const DirectX::VertexPositionColor start(line.start, line.color);
			const DirectX::VertexPositionColor end(line.end, line.color);
			_dxResources->primitiveBatch->DrawLine(start, end);
		}


		for (const DebugRay& ray : _rays)
		{
			Vector3 direction = ray.ray.direction;
			direction.Normalize();
			debug_draw::DrawRay(_dxResources->primitiveBatch.get(), ray.ray.position, direction * ray.length, false, ray.color);
		}

		_dxResources->primitiveBatch->End();

		Clear();
#endif
	}

	void D3D11DebugRenderer::RequestDrawLine(const Vector2& start, const Vector2& end, Color color)
	{
		RequestDrawLine(Vector3(start.x, start.y, 0.f), Vector3(end.x, end.y, 0.f), color);
	}

	void D3D11DebugRenderer::RequestDrawLine(const Vector3& start, const Vector3& end, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_lines.push_back({ start, end, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawRect(const Vector2& center, const Vector2& size, Color color)
	{
		const Vector2 halfSize = size * 0.5f;
		const Vector3 leftTop(center.x - halfSize.x, center.y + halfSize.y, 0.f);
		const Vector3 rightTop(center.x + halfSize.x, center.y + halfSize.y, 0.f);
		const Vector3 rightBottom(center.x + halfSize.x, center.y - halfSize.y, 0.f);
		const Vector3 leftBottom(center.x - halfSize.x, center.y - halfSize.y, 0.f);

		RequestDrawQuad(leftTop, rightTop, rightBottom, leftBottom, color);
	}

	void D3D11DebugRenderer::RequestDrawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_triangles.push_back({ a, b, c, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_quads.push_back({ a, b, c, d, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawCircle(const Vector2& center, float radius, Color color, uint32 segments)
	{
		RequestDrawCircle(Vector3(center.x, center.y, 0.f), radius, color, segments);
	}

	void D3D11DebugRenderer::RequestDrawCircle(const Vector3& center, float radius, Color color, uint32 segments)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_rings.push_back({ center, Vector3(radius, 0.f, 0.f), Vector3(0.f, radius, 0.f), color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawBox(const Vector3& center, const Vector3& extent, Color color)
	{
		RequestDrawBox(BoundingBox(center, extent), color);
	}

	void D3D11DebugRenderer::RequestDrawBox(const BoundingBox& box, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_boxes.push_back({ box, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawOBB(const Vector3& center, const Vector3& extents, const Quaternion& quaternion, Color color)
	{
		RequestDrawOBB({ center, extents, quaternion }, color);
	}

	void D3D11DebugRenderer::RequestDrawOBB(const BoundingOrientedBox& obb, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_obbs.push_back({ obb, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawSphere(const Vector3& center, float radius, Color color, uint32 segments)
	{
		RequestDrawSphere(BoundingSphere(center, radius), color);
	}

	void D3D11DebugRenderer::RequestDrawSphere(const BoundingSphere& sphere, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_spheres.push_back({ sphere, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawRay(const Vector3& origin, const Vector3& direction, float length, Color color)
	{
		RequestDrawRay(Ray(origin, direction), length, color);
	}

	void D3D11DebugRenderer::RequestDrawRay(const Ray& ray, float length, Color color)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		if (ray.direction.LengthSquared() <= 0.f || length <= 0.f)
			return;

		_rays.push_back({ ray, length, color });
#endif
	}

	void D3D11DebugRenderer::RequestDrawText(const std::wstring& content, const Vector2& viewPosition, float fontSize, Color color,
		TextHorizontalAlignment horizontalAlignment, TextVerticalAlignment verticalAlignment)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (IsEnabled() == false)
			return;

		_textRenderer->RequestDrawText(content, BuiltinResourceKey::DefaultUIFont, viewPosition, fontSize, color, horizontalAlignment, verticalAlignment);
#endif
	}

	void D3D11DebugRenderer::Clear()
	{
#if GM_ENABLE_DEBUG_TOOLS
		_lines.clear();
		_triangles.clear();
		_quads.clear();
		_rings.clear();
		_boxes.clear();
		_obbs.clear();
		_spheres.clear();
		_rays.clear();
#endif
	}
}
