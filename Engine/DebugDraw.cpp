//--------------------------------------------------------------------------------------
// File: DebugDraw.cpp
//
// Based on the Microsoft DirectXTK DebugDraw sample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#include "DebugDraw.h"
#include <algorithm>
#include <cstdint>
#include <cmath>

using namespace DirectX;

namespace
{
	template <typename T, size_t N>
	constexpr size_t CountOf(const T(&)[N])
	{
		return N;
	}

	inline void XM_CALLCONV DrawCube(PrimitiveBatch<VertexPositionColor>* batch, CXMMATRIX matWorld, FXMVECTOR color)
	{
		static const XMVECTORF32 verts[8] =
		{
			{ { { -1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f,  1.f, 0.f } } }
		};

		static const std::uint16_t indices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		VertexPositionColor transformedVerts[8];
		for (size_t i = 0; i < CountOf(verts); ++i)
		{
			const XMVECTOR position = XMVector3Transform(verts[i], matWorld);
			XMStoreFloat3(&transformedVerts[i].position, position);
			XMStoreFloat4(&transformedVerts[i].color, color);
		}

		batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, indices, CountOf(indices), transformedVerts, CountOf(transformedVerts));
	}
}

void XM_CALLCONV gm::debug_draw::Draw(PrimitiveBatch<VertexPositionColor>* batch, const BoundingSphere& sphere, FXMVECTOR color)
{
	const XMVECTOR origin = XMLoadFloat3(&sphere.Center);
	const float radius = sphere.Radius;

	const XMVECTOR xAxis = g_XMIdentityR0 * radius;
	const XMVECTOR yAxis = g_XMIdentityR1 * radius;
	const XMVECTOR zAxis = g_XMIdentityR2 * radius;

	DrawRing(batch, origin, xAxis, zAxis, color);
	DrawRing(batch, origin, xAxis, yAxis, color);
	DrawRing(batch, origin, yAxis, zAxis, color);
}

void XM_CALLCONV gm::debug_draw::Draw(PrimitiveBatch<VertexPositionColor>* batch, const BoundingBox& box, FXMVECTOR color)
{
	XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
	const XMVECTOR position = XMLoadFloat3(&box.Center);
	matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

void XM_CALLCONV gm::debug_draw::Draw(PrimitiveBatch<VertexPositionColor>* batch, const BoundingOrientedBox& obb, FXMVECTOR color)
{
	XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
	const XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
	matWorld = XMMatrixMultiply(matScale, matWorld);

	const XMVECTOR position = XMLoadFloat3(&obb.Center);
	matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

void XM_CALLCONV gm::debug_draw::Draw(PrimitiveBatch<VertexPositionColor>* batch, const BoundingFrustum& frustum, FXMVECTOR color)
{
	XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
	frustum.GetCorners(corners);

	VertexPositionColor verts[24] = {};
	verts[0].position = corners[0];
	verts[1].position = corners[1];
	verts[2].position = corners[1];
	verts[3].position = corners[2];
	verts[4].position = corners[2];
	verts[5].position = corners[3];
	verts[6].position = corners[3];
	verts[7].position = corners[0];

	verts[8].position = corners[0];
	verts[9].position = corners[4];
	verts[10].position = corners[1];
	verts[11].position = corners[5];
	verts[12].position = corners[2];
	verts[13].position = corners[6];
	verts[14].position = corners[3];
	verts[15].position = corners[7];

	verts[16].position = corners[4];
	verts[17].position = corners[5];
	verts[18].position = corners[5];
	verts[19].position = corners[6];
	verts[20].position = corners[6];
	verts[21].position = corners[7];
	verts[22].position = corners[7];
	verts[23].position = corners[4];

	for (VertexPositionColor& vert : verts)
		XMStoreFloat4(&vert.color, color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, CountOf(verts));
}

void XM_CALLCONV gm::debug_draw::DrawGrid(PrimitiveBatch<VertexPositionColor>* batch,
	FXMVECTOR xAxis,
	FXMVECTOR yAxis,
	FXMVECTOR origin,
	size_t xdivs,
	size_t ydivs,
	GXMVECTOR color)
{
	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float percent = static_cast<float>(i) / static_cast<float>(xdivs);
		percent = percent * 2.f - 1.f;
		XMVECTOR scale = XMVectorScale(xAxis, percent);
		scale = XMVectorAdd(scale, origin);

		const VertexPositionColor v1(XMVectorSubtract(scale, yAxis), color);
		const VertexPositionColor v2(XMVectorAdd(scale, yAxis), color);
		batch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; ++i)
	{
		float percent = static_cast<float>(i) / static_cast<float>(ydivs);
		percent = percent * 2.f - 1.f;
		XMVECTOR scale = XMVectorScale(yAxis, percent);
		scale = XMVectorAdd(scale, origin);

		const VertexPositionColor v1(XMVectorSubtract(scale, xAxis), color);
		const VertexPositionColor v2(XMVectorAdd(scale, xAxis), color);
		batch->DrawLine(v1, v2);
	}
}

void XM_CALLCONV gm::debug_draw::DrawRing(PrimitiveBatch<VertexPositionColor>* batch,
	FXMVECTOR origin,
	FXMVECTOR majorAxis,
	FXMVECTOR minorAxis,
	GXMVECTOR color)
{
	static constexpr size_t RingSegments = 32;

	VertexPositionColor verts[RingSegments + 1];

	const float angleDelta = XM_2PI / static_cast<float>(RingSegments);
	const XMVECTOR cosDelta = XMVectorReplicate(std::cos(angleDelta));
	const XMVECTOR sinDelta = XMVectorReplicate(std::sin(angleDelta));

	XMVECTOR incrementalSin = XMVectorZero();
	static const XMVECTORF32 initialCos = { { { 1.f, 1.f, 1.f, 1.f } } };
	XMVECTOR incrementalCos = initialCos.v;

	for (size_t i = 0; i < RingSegments; ++i)
	{
		XMVECTOR position = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
		position = XMVectorMultiplyAdd(minorAxis, incrementalSin, position);
		XMStoreFloat3(&verts[i].position, position);
		XMStoreFloat4(&verts[i].color, color);

		const XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
		const XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
		incrementalCos = newCos;
		incrementalSin = newSin;
	}
	verts[RingSegments] = verts[0];

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, RingSegments + 1);
}

void XM_CALLCONV gm::debug_draw::DrawRay(PrimitiveBatch<VertexPositionColor>* batch,
	FXMVECTOR origin,
	FXMVECTOR direction,
	bool normalize,
	FXMVECTOR color)
{
	VertexPositionColor verts[3];
	XMStoreFloat3(&verts[0].position, origin);

	XMVECTOR normDirection = XMVector3Normalize(direction);
	XMVECTOR rayDirection = normalize ? normDirection : direction;
	XMVECTOR perpVector = XMVector3Cross(normDirection, g_XMIdentityR1);

	if (XMVector3Equal(XMVector3LengthSq(perpVector), g_XMZero))
		perpVector = XMVector3Cross(normDirection, g_XMIdentityR2);

	perpVector = XMVector3Normalize(perpVector);

	XMStoreFloat3(&verts[1].position, XMVectorAdd(rayDirection, origin));
	perpVector = XMVectorScale(perpVector, 0.0625f);
	normDirection = XMVectorScale(normDirection, -0.25f);
	rayDirection = XMVectorAdd(perpVector, rayDirection);
	rayDirection = XMVectorAdd(normDirection, rayDirection);
	XMStoreFloat3(&verts[2].position, XMVectorAdd(rayDirection, origin));

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 3);
}

void XM_CALLCONV gm::debug_draw::DrawTriangle(PrimitiveBatch<VertexPositionColor>* batch,
	FXMVECTOR pointA,
	FXMVECTOR pointB,
	FXMVECTOR pointC,
	GXMVECTOR color)
{
	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, pointA);
	XMStoreFloat3(&verts[1].position, pointB);
	XMStoreFloat3(&verts[2].position, pointC);
	XMStoreFloat3(&verts[3].position, pointA);

	for (VertexPositionColor& vert : verts)
		XMStoreFloat4(&vert.color, color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, CountOf(verts));
}

void XM_CALLCONV gm::debug_draw::DrawQuad(PrimitiveBatch<VertexPositionColor>* batch,
	FXMVECTOR pointA,
	FXMVECTOR pointB,
	FXMVECTOR pointC,
	GXMVECTOR pointD,
	HXMVECTOR color)
{
	VertexPositionColor verts[5];
	XMStoreFloat3(&verts[0].position, pointA);
	XMStoreFloat3(&verts[1].position, pointB);
	XMStoreFloat3(&verts[2].position, pointC);
	XMStoreFloat3(&verts[3].position, pointD);
	XMStoreFloat3(&verts[4].position, pointA);

	for (VertexPositionColor& vert : verts)
		XMStoreFloat4(&vert.color, color);

	batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, CountOf(verts));
}
