#pragma once

#include "EffectPresetData.h"
#include "GameObject.h"
#include "WeakGameObjectPtr.h"

#include <memory>
#include <optional>

namespace gm
{
	class EffectSpriteComponent;
	class Shader;
	class SocketComponent;
	class StaticMesh;
	class StaticMeshComponent;
	class Texture;

	enum class EffectAttachmentMode
	{
		World,
		Owner,
		OwnerPosition,
		SocketPosition,
		Socket,

		Count,
	};

	struct EffectRuntimeObjectDesc
	{
		EffectTrackData				track{};
		EffectAttachmentMode		attachmentMode = EffectAttachmentMode::World;
		Matrix						spawnTransform = Matrix::Identity;
		Vector3					ownerSpawnPosition{};
		WeakGameObjectPtr			owner{};
		std::wstring				socketName{};
		std::shared_ptr<Texture>	spriteTexture{};
		std::shared_ptr<StaticMesh> staticMesh{};
		std::shared_ptr<Texture>	baseColorTexture{};
		std::shared_ptr<Texture>	dissolveTexture{};
		std::shared_ptr<Shader>		effectMeshShader{};
	};

	class EffectRuntimeObject final : public GameObject
	{
		friend class EffectInstance;

	public:
		explicit EffectRuntimeObject(const EffectRuntimeObjectDesc& desc);
		void SetWorldTransform(const Matrix& world);

	protected:
		void	OnInitialize() override;
		void	OnTick(float deltaTime) override;

	private:
		bool	ConfigureVisual(const EffectRuntimeObjectDesc& desc);
		void	UpdateTransformAnimation(float deltaTime);
		bool	UpdateAttachmentTransform();
		void	UpdateElapsedTime(float deltaTime);
		void	UpdatePresentation(float deltaTime);
		void	UpdateSpriteSheetFrame();
		void	OverrideOpacity(float opacity);
		void	SetOpacity(float opacity);
		void	SetEmissiveIntensity(float intensity);
		void	SetDissolveThreshold(float threshold);
		float	CalculateRotationCircleAngle() const;
		float	GetActiveRatio() const;
		void	Activate();

		EffectTime				_delay{};
		EffectTime				_lifetime{};
		EffectAttachmentMode	_attachmentMode = EffectAttachmentMode::World;
		Matrix					_localTransform = Matrix::Identity;
		Matrix					_spawnTransform = Matrix::Identity;
		Vector3					_ownerSpawnPosition{};
		Vector3					_startScale{ 1.f, 1.f, 1.f };
		Vector3					_endScale{ 1.f, 1.f, 1.f };
		Vector3					_endPositionOffset{};
		Vector3					_endRotationOffsetDegrees{};
		Vector3					_currentScale{ 1.f, 1.f, 1.f };
		Vector3					_currentPositionOffset{};
		Vector3					_currentRotationOffsetDegrees{};
		Vector3					_randomPositionOffset{};
		Vector3					_randomRotationOffsetDegrees{};
		float					_positionInterpolationSpeed = 0.f;
		float					_rotationInterpolationSpeed = 0.f;
		float					_scaleInterpolationSpeed = 0.f;
		EffectTime				_scaleInterpolationDelay{};
		Vector3					_startAngularVelocityDegrees{};
		Vector3					_endAngularVelocityDegrees{};
		Vector3					_currentAngularVelocityDegrees{};
		float					_angularVelocityInterpolationSpeed = 0.f;
		EffectAnimationType	_animationType = EffectAnimationType::Interpolation;
		float					_animationSpeed = 1.f;
		WeakGameObjectPtr		_owner{};
		const SocketComponent*	_socketComponent = nullptr;
		EffectSpriteComponent*	_spriteComponent = nullptr;
		StaticMeshComponent*	_staticMeshComponent = nullptr;
		std::wstring			_socketName{};
		uint32				_spriteSheetColumns = 1;
		uint32				_spriteSheetRows = 1;
		uint32				_spriteSheetStartFrame = 0;
		uint32				_spriteSheetFrameCount = 1;
		uint32				_currentSpriteSheetFrame = static_cast<uint32>(-1);
		EffectTime			_spriteSheetFrameDuration{};
		float					_startOpacity = 1.f;
		float					_endOpacity = 1.f;
		float					_currentOpacity = 1.f;
		float					_opacityInterpolationSpeed = 0.f;
		float					_startFillRatio = 1.f;
		float					_endFillRatio = 1.f;
		Vector2					_radialCenter{ 0.5f, 0.5f };
		float					_radialStartAngle = 0.f;
		float					_radialSweepAngle = 0.f;
		float					_dissolveStartThreshold = 0.f;
		float					_dissolveEndThreshold = 0.f;
		EffectTime				_emissiveDuration{};
		EffectMaterialConstantPS _effectMaterialConstant{};
		float					_elapsedSeconds = 0.f;
		float					_elapsedBeats = 0.f;
		float					_activeElapsedSeconds = 0.f;
		float					_activeElapsedBeats = 0.f;
		std::optional<float>	_previousBeat{};
		bool					_isActive = false;
		bool					_isConfigured = false;
		bool					_hasDissolve = false;
	};
}
