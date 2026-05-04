#pragma once

#include "Component.h"

namespace gm
{
	enum class Collider2DType
	{
		Box,
		Circle,
	};

	class Collider2D : public Component
	{
	public:
		virtual ~Collider2D() = default;

		Collider2DType				GetColliderType() const { return _colliderType; }
		void						SetOffset(const Vector2& offset) { _offset = offset; }
		const Vector2&				GetOffset() const { return _offset; }

		void						SetTrigger(bool isTrigger) { _isTrigger = isTrigger; }
		bool						IsTrigger() const { return _isTrigger; }

	protected:
		explicit Collider2D(Collider2DType colliderType) : _colliderType(colliderType) {}

		void						OnLateUpdate() override;

	private:
		Collider2DType				_colliderType;
		Vector2						_offset{};
		bool						_isTrigger = false;
	};
}
