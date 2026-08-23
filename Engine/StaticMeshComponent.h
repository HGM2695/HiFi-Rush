#pragma once

#include "BoundingTypes.h"
#include "Component.h"
#include "MathTypes.h"
#include <memory>
#include <vector>

namespace gm
{
	class Material;
	class StaticMesh;
	class TransformComponent;

	class StaticMeshComponent : public Component
	{
	public:
		StaticMeshComponent();
		virtual ~StaticMeshComponent();

		void								SetStaticMesh(const std::shared_ptr<StaticMesh>& staticMesh);
		const std::shared_ptr<StaticMesh>&	GetStaticMesh() const { return _staticMesh; }

		void								SetPreTransform(const Matrix& preTransform) { _preTransform = preTransform; }
		const Matrix&						GetPreTransform() const { return _preTransform; }
		BoundingVolume						GetLocalBounds() const;
		BoundingVolume						GetWorldBounds() const;

		void								SetMaterial(uint32 slotIndex, const Material& material);
		Material*							GetMaterial(uint32 slotIndex) const;
		void								SetCastsShadow(bool castsShadow) { _castsShadow = castsShadow; }
		bool								CastsShadow() const { return _castsShadow; }

	protected:
		virtual void OnInitialize() override;
		virtual void OnRender() override;

	private:
		TransformComponent*						_ownerTransform = nullptr;
		Matrix									_preTransform = Matrix::CreateScale(1.f);

		std::shared_ptr<StaticMesh>				_staticMesh;
		std::vector<std::unique_ptr<Material>>	_materials;
		bool									_castsShadow = true;
	};
}
