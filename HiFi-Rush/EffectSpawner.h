#pragma once

#include "MathTypes.h"

#include <string>

namespace gm
{
	class GameObject;
	class EffectInstance;
	class EffectPresets;
	class Resources;
	class Scene;
	enum class EffectAttachmentMode;
	struct EffectRuntimeObjectDesc;
	struct EffectTrackData;

	class EffectSpawner
	{
	public:
		EffectSpawner(Resources& resources, const EffectPresets& effectPresets);

		bool SpawnAtWorld(Scene& scene, const std::wstring& effectId, const Matrix& world) const;
		bool SpawnAtWorld(Scene& scene, const std::wstring& effectId, const Matrix& world, _Out_ EffectInstance& outInstance) const;
		bool SpawnAttachedToOwner(Scene& scene, const std::wstring& effectId, GameObject& owner, const Matrix& localTransform = Matrix::Identity) const;
		bool SpawnAttachedToSocket(Scene& scene, const std::wstring& effectId, GameObject& owner, const std::wstring& socketName, const Matrix& localTransform = Matrix::Identity) const;

	private:
		bool Spawn(Scene& scene, const std::wstring& effectId, EffectAttachmentMode attachmentMode, const Matrix& spawnTransform, GameObject* owner, const std::wstring& socketName, EffectInstance* outInstance = nullptr) const;
		bool ValidateAttachment(EffectAttachmentMode attachmentMode, GameObject* owner, const std::wstring& socketName) const;
		bool CreateRuntimeObjectDesc(const EffectTrackData& track, EffectAttachmentMode attachmentMode, const Matrix& spawnTransform, GameObject* owner, const std::wstring& socketName, _Out_ EffectRuntimeObjectDesc& outDesc) const;

		Resources& _resources;
		const EffectPresets& _effectPresets;
	};
}
