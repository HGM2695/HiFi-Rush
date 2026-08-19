#pragma once

#include "Scene.h"

#include <future>
#include <string>
#include <vector>

namespace gm
{
	class IGraphicsResourceFactory;
	class Resource;
	class Resources;
	enum class MonsterType : uint32;
	struct RhythmBGMDesc;

	class CommonLoadingScene : public Scene
	{
	protected:
		void OnInitialize() override;
		void OnEnter() override;
		void OnExit() override;
		void OnTick(float deltaTime) override;
		void OnRender() override;

	private:
		struct ResourceLoadData
		{
			std::wstring				key;
			std::shared_ptr<Resource>	resource;
		};

		struct SceneLoadData
		{
			std::vector<ResourceLoadData>	resources;
			std::wstring					errorMessage;
			bool							succeeded = false;
		};

		enum class LoadingState
		{
			Idle,
			LoadingResources,
			ReadyToTransition,
			TransitionRequested,
			Failed,
		};

		static SceneLoadData	LoadPendingSceneResources(const std::wstring& sceneName, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static SceneLoadData	LoadTutorialSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static SceneLoadData	LoadOutsideSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static SceneLoadData	LoadQamilSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadGameplayUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadTutorialUIResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadDialogResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadMeshTextures(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadMapResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory, const std::wstring& mapFileName);
		static bool				LoadChiResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory);
		static bool				LoadMonsterResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory, MonsterType monsterType);
		static bool				LoadNavigationMesh(SceneLoadData& outLoadData, Resources& resources, const std::wstring& key, const std::wstring& fileName);
		static bool				LoadSoundWave(SceneLoadData& outLoadData, Resources& resources, const std::wstring& key, const std::wstring& fileName);
		static bool				LoadRhythmBGM(SceneLoadData& outLoadData, Resources& resources, const RhythmBGMDesc& desc);

		bool					RegisterLoadedResources();
		void					RequestTargetSceneTransition();
		float					GetLoadingProgress() const;

	private:
		std::future<SceneLoadData>			_loadFuture{};
		SceneLoadData						_sceneLoadData{};
		std::wstring						_pendingSceneName{};
		float								_accLoadingTime = 0.f;
		float								_minimumLoadingTime = 2.f;
		LoadingState						_loadingState = LoadingState::Idle;
	};
}
