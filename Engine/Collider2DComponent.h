#pragma once

#include "Component.h"

namespace gm
{
	enum class ColliderShape2DType
	{
		Box,
		Circle,
	};

	class Collider2DComponent : public Component
	{
	public:
		virtual ~Collider2DComponent() = default;

		ColliderShape2DType		GetShapeType() const { return _shapeType; }
		void						SetOffset(const Vector2& offset) { _offset = offset; }
		const Vector2&				GetOffset() const { return _offset; }

		void						SetTrigger(bool isTrigger) { _isTrigger = isTrigger; }
		bool						IsTrigger() const { return _isTrigger; }

	protected:
		explicit Collider2DComponent(ColliderShape2DType shapeType) : _shapeType(shapeType) {}

		void						OnTick(float deltaTime) override;

	private:
		ColliderShape2DType		_shapeType;
		Vector2						_offset{};
		bool						_isTrigger = false;
	};
}
