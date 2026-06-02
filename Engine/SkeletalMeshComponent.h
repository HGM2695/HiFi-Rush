#pragma once

#include "Component.h"
#include "MathTypes.h"
#include "SkeletalAnimator.h"
#include "SkeletalPose.h"

#include <memory>
#include <vector>

namespace gm
{
	class Material;
	class SkeletalMesh;
	class TransformComponent;

	class SkeletalMeshComponent : public Component
	{
	public:
		SkeletalMeshComponent();
		virtual ~SkeletalMeshComponent();

		void									SetPreTransform(const Matrix& preTransform) { _preTransform = preTransform; }
		const Matrix&							GetPreTransform() const { return _preTransform; }

		SkeletalAnimator&						GetAnimator() { return _animator; }
		const SkeletalAnimator&					GetAnimator() const { return _animator; }
		SkeletalPose&							GetPose() { return _pose; }
		const SkeletalPose&						GetPose() const { return _pose; }

		void									SetSkeletalMesh(const std::shared_ptr<SkeletalMesh>& skeletalMesh);
		const std::shared_ptr<SkeletalMesh>&	GetSkeletalMesh() const { return _skeletalMesh; }

		void									SetMaterial(uint32 slotIndex, const Material& material);
		Material* GetMaterial(uint32 slotIndex) const;

	protected:
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;
		virtual void OnRender() override;

	private:
		TransformComponent*						_ownerTransform = nullptr;
		Matrix									_preTransform = Matrix::CreateScale(1.f);

		SkeletalAnimator						_animator;
		// 원본
		std::shared_ptr<SkeletalMesh>			_skeletalMesh;
		// 변환된 결과물
		SkeletalPose							_pose;
		std::vector<std::unique_ptr<Material>>	_materials;
	};
}
