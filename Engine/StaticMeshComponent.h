#pragma once

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

		virtual TickGroup GetTickGroup() const override { return TickGroup::RenderSubmit; }

		void								SetStaticMesh(const std::shared_ptr<StaticMesh>& staticMesh);
		const std::shared_ptr<StaticMesh>&	GetStaticMesh() const { return _staticMesh; }

		void								SetPreTransform(const Matrix& preTransform) { _preTransform = preTransform; }
		const Matrix&						GetPreTransform() const { return _preTransform; }

		void								SetMaterial(uint32 slotIndex, const Material& material);
		Material*							GetMaterial(uint32 slotIndex) const;

	protected:
		virtual void OnInitialize() override;
		virtual void OnRender() override;

	private:
		TransformComponent*						_ownerTransform = nullptr;

		std::shared_ptr<StaticMesh>				_staticMesh;
		std::vector<std::unique_ptr<Material>>	_materials;
		Matrix									_preTransform = Matrix::CreateScale(1.f);
	};
}
