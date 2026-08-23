#pragma once

#include "GameObject.h"

namespace gm
{
	class SkeletalMeshComponent;

	struct ChiAfterImageObjectDesc
	{
		const SkeletalMeshComponent* sourceMeshComponent = nullptr;
		Matrix	world = Matrix::Identity;
		Color	color = Colors::White;
		float	lifetime = 0.2f;
		float	emissiveIntensity = 0.f;
		float	colorBlendRatio = 0.5f;
		bool	fadeOut = true;
	};

	class ChiAfterImageObject final : public GameObject
	{
	public:
		explicit ChiAfterImageObject(const ChiAfterImageObjectDesc& desc);
		void UpdateSnapshot(const SkeletalMeshComponent& sourceMeshComponent, const Matrix& world);
		void SetColor(const Color& color);
		void SetEmissiveIntensity(float intensity);
		void SetColorBlendRatio(float ratio);
		void Finish();

	protected:
		void OnTick(float deltaTime) override;

	private:
		void UpdateOpacity(float opacity);

		SkeletalMeshComponent* _meshComponent = nullptr;
		Color	_color = Colors::White;
		float	_colorBlendRatio = 0.5f;
		float	_lifetime = 0.2f;
		float	_elapsed = 0.f;
		bool	_fadeOut = true;
	};
}
