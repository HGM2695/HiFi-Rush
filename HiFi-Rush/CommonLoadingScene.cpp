#include "CommonLoadingScene.h"
#include "Application.h"
#include "BinaryEnvironmentMapLoader.h"
#include "BinaryModelLoader.h"
#include "BinaryNavigationMeshLoader.h"
#include "BuiltinGraphicsResources.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "ChiAnimationTypes.h"
#include "EnvironmentMapTypes.h"
#include "GameObject.h"
#include "IGraphicsResourceFactory.h"
#include "ITextRenderer.h"
#include "LoadingScreenWidget.h"
#include "NavigationMesh.h"
#include "PathUtil.h"
#include "Paths.h"
#include "Resources.h"
#include "SceneManager.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "StaticMesh.h"
#include "Texture.h"
#include "UIManager.h"

#include <algorithm>
#include <chrono>
#include <filesystem>

namespace gm
{
	void CommonLoadingScene::OnInitialize()
	{
		GameObject* cameraObject = SpawnGameObject<GameObject>();
		CameraComponent* camera = cameraObject->AddComponent<CameraComponent>();
		GetCameraManager()->RegisterCamera(L"LoadingCamera", camera);
	}

	void CommonLoadingScene::OnEnter()
	{
		_pendingSceneName = APPLICATION.GetSceneManager().GetPendingSceneName();
		GM_ASSERT_RETURN(_pendingSceneName.empty() == false, "LoadingScene에 목표 Scene이 지정되지 않았습니다.");

		UIManager& uiManager = APPLICATION.GetUIManager();
		uiManager.ClearViewportWidgets();
		uiManager.AddUserWidget<LoadingScreenWidget>();

		_sceneLoadData = {};
		_accLoadingTime = 0.f;

		const std::wstring targetSceneName = _pendingSceneName;
		Resources* resources = &APPLICATION.GetResources();
		IGraphicsResourceFactory* resourceFactory = &APPLICATION.GetGraphicsResourceFactory();
		// 워커가 완료될 때까지 Resources는 조회만 하고, 등록은 메인 스레드에서 수행합니다.
		_loadFuture = std::async(std::launch::async, [targetSceneName, resources, resourceFactory]
			{
				return LoadPendingSceneResources(targetSceneName, *resources, *resourceFactory);
			});
		_loadingState = LoadingState::LoadingResources;
	}

	void CommonLoadingScene::OnExit()
	{
		APPLICATION.GetUIManager().ClearViewportWidgets();
	}

	void CommonLoadingScene::OnTick(float deltaTime)
	{
		_accLoadingTime += deltaTime;

		if (_loadingState == LoadingState::LoadingResources)
		{
			if (_loadFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				return;

			_sceneLoadData = _loadFuture.get();
			if (_sceneLoadData.succeeded == false)
			{
				_loadingState = LoadingState::Failed;
				GM_ASSERT_RETURN(false, "Scene 리소스 로딩에 실패했습니다. scene=%ls, reason=%ls", _pendingSceneName.c_str(), _sceneLoadData.errorMessage.c_str());
			}

			if (RegisterLoadedResources() == false)
			{
				_loadingState = LoadingState::Failed;
				return;
			}

			_sceneLoadData = {};
			_loadingState = LoadingState::ReadyToTransition;
		}

		if (_loadingState == LoadingState::ReadyToTransition && _accLoadingTime >= _minimumLoadingTime)
			RequestTargetSceneTransition();
	}

	void CommonLoadingScene::OnRender()
	{
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadPendingSceneResources(const std::wstring& sceneName, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		if (sceneName == L"TutorialScene")
			return LoadTutorialSceneResources(resources, resourceFactory);

		if (sceneName == L"OutsideScene")
			return LoadOutsideSceneResources(resources, resourceFactory);

		if (sceneName == L"QamilScene")
			return LoadQamilSceneResources(resources, resourceFactory);

		SceneLoadData result{};
		result.errorMessage = L"지원하지 않는 Scene 로딩 요청입니다.";
		return result;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadTutorialSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadEnvironmentResources(result, resources, resourceFactory, L"TutorialEnvironmentMap.bin") == false)
			return result;

		if (LoadChiResources(result, resources, resourceFactory) == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"tutorial", L"tutorial.bin") == false)
			return result;

		result.succeeded = true;
		return result;
	}

	bool CommonLoadingScene::LoadMeshTextures(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		const std::filesystem::path meshTexturePath = GetTexturePath(L"Mesh");
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(meshTexturePath))
		{
			if (entry.is_regular_file() == false)
				continue;

			const std::wstring textureKey = GetFileNameWithoutExtension(entry.path().wstring());
			if (textureKey.empty())
				continue;

			if (resources.Find<Texture>(textureKey))
				continue;

			TextureDesc desc{};
			desc.path = entry.path().wstring();

			std::shared_ptr<Texture> texture = resourceFactory.CreateTexture(desc);
			if (texture == nullptr)
			{
				outLoadData.errorMessage = L"환경 Texture 생성에 실패했습니다. key=" + textureKey;
				return false;
			}

			outLoadData.resources.push_back({ textureKey, std::move(texture) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadEnvironmentResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory, const std::wstring& mapFileName)
	{
		EnvironmentMapData mapData{};
		if (BinaryEnvironmentMapLoader::Load(GetMapPath(mapFileName), mapData) == false)
		{
			outLoadData.errorMessage = L"환경 맵 데이터 로드에 실패했습니다.";
			return false;
		}

		std::vector<uint32> modelIndices;
		modelIndices.reserve(mapData.objects.size());
		for (const EnvironmentObjectData& object : mapData.objects)
			modelIndices.push_back(object.modelIndex);

		// 로딩 순서 보장을 위한 작업
		std::sort(modelIndices.begin(), modelIndices.end());
		modelIndices.erase(std::unique(modelIndices.begin(), modelIndices.end()), modelIndices.end());

		BinaryModelLoader loader;
		for (uint32 modelIndex : modelIndices)
		{
			const std::wstring modelKey = L"Environment" + std::to_wstring(modelIndex);
			if (resources.Find<StaticMesh>(modelKey) || resources.Find<SkeletalMesh>(modelKey))
				continue;

			const std::wstring modelPath = GetModelPath(L"Binary/Environment/" + modelKey + L".bin");
			ModelData modelData = loader.Load(modelPath);

			const bool hasValidMeshData =
				modelData.type == ModelType::Static
				? modelData.vertices.empty() == false
				: modelData.skinnedVertices.empty() == false;

			if (hasValidMeshData == false || modelData.indices.empty())
			{
				outLoadData.errorMessage = L"환경 모델 데이터가 유효하지 않습니다. key=" + modelKey;
				return false;
			}

			if (modelData.type == ModelType::Static)
			{
				std::shared_ptr<StaticMesh> staticMesh = StaticMesh::Create(modelData, resourceFactory);
				if (staticMesh == nullptr)
				{
					outLoadData.errorMessage = L"환경 StaticMesh 생성에 실패했습니다. key=" + modelKey;
					return false;
				}

				outLoadData.resources.push_back({ modelKey, std::move(staticMesh) });
				continue;
			}

			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"환경 SkeletalMesh 생성에 실패했습니다. key=" + modelKey;
				return false;
			}

			outLoadData.resources.push_back({ modelKey, std::move(skeletalMesh) });

			for (uint32 animationIndex = 0; animationIndex < modelData.animations.size(); ++animationIndex)
			{
				const SkeletalAnimationClipData& clipData = modelData.animations[animationIndex];
				std::shared_ptr<SkeletalAnimationClip> clip = SkeletalAnimationClip::Create(clipData);
				if (clip == nullptr)
				{
					outLoadData.errorMessage = L"환경 SkeletalAnimationClip 생성에 실패했습니다. model=" + modelKey;
					return false;
				}

				const std::wstring animationKey = modelKey + L"." + clipData.name;
				outLoadData.resources.push_back({ animationKey, clip });

				if (animationIndex == 0)
					outLoadData.resources.push_back({ modelKey + L".DefaultAnimation", std::move(clip) });
			}
		}

		return true;
	}

	bool CommonLoadingScene::LoadChiResources(SceneLoadData& outLoadData, Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		bool hasAllResources = resources.Find<SkeletalMesh>(L"chi") != nullptr;
		hasAllResources &= resources.Find<SkeletalAnimationClip>(L"chi.DefaultAnimation") != nullptr;
		for (uint32 animationIndex = 0; animationIndex < ChiAnimationIdCount; ++animationIndex)
			hasAllResources &= resources.Find<SkeletalAnimationClip>(GetChiAnimationKey(static_cast<ChiAnimationId>(animationIndex))) != nullptr;

		if (hasAllResources)
			return true;

		BinaryModelLoader loader;
		ModelData modelData = loader.Load(GetModelPath(L"Binary/Characters/Chi.bin"));

		if (resources.Find<SkeletalMesh>(L"chi") == nullptr)
		{
			std::shared_ptr<SkeletalMesh> skeletalMesh = SkeletalMesh::Create(modelData, resourceFactory);
			if (skeletalMesh == nullptr)
			{
				outLoadData.errorMessage = L"Chi SkeletalMesh 생성에 실패했습니다.";
				return false;
			}

			outLoadData.resources.push_back({ L"chi", std::move(skeletalMesh) });
		}

		if (modelData.animations.size() != ChiAnimationIdCount)
		{
			outLoadData.errorMessage = L"Chi 애니메이션 개수가 ChiAnimationId와 일치하지 않습니다.";
			return false;
		}

		for (uint32 animationIndex = 0; animationIndex < modelData.animations.size(); ++animationIndex)
		{
			const SkeletalAnimationClipData& clipData = modelData.animations[animationIndex];
			const ChiAnimationId animationId = static_cast<ChiAnimationId>(animationIndex);
			const std::wstring animationKey = GetChiAnimationKey(animationId);
			std::shared_ptr<SkeletalAnimationClip> clip = resources.Find<SkeletalAnimationClip>(animationKey);
			if (clip == nullptr)
			{
				clip = SkeletalAnimationClip::Create(clipData);
				if (clip == nullptr)
				{
					outLoadData.errorMessage = L"Chi SkeletalAnimationClip 생성에 실패했습니다.";
					return false;
				}

				outLoadData.resources.push_back({ animationKey, clip });
			}

			if (animationId == ChiAnimationId::Idle && resources.Find<SkeletalAnimationClip>(L"chi.DefaultAnimation") == nullptr)
				outLoadData.resources.push_back({ L"chi.DefaultAnimation", std::move(clip) });
		}

		return true;
	}

	bool CommonLoadingScene::LoadNavigationMesh(SceneLoadData& outLoadData, Resources& resources, const std::wstring& key, const std::wstring& fileName)
	{
		if (resources.Find<NavigationMesh>(key))
			return true;

		NavigationMeshData navigationMeshData{};
		if (BinaryNavigationMeshLoader::Load(GetResourcePath(L"NavigationMesh/" + fileName), navigationMeshData) == false)
		{
			outLoadData.errorMessage = key + L" NavigationMesh 데이터 로드에 실패했습니다.";
			return false;
		}

		std::shared_ptr<NavigationMesh> navigationMesh = NavigationMesh::Create(NavigationMeshDesc{ navigationMeshData });
		if (navigationMesh == nullptr)
		{
			outLoadData.errorMessage = key + L" NavigationMesh 생성에 실패했습니다.";
			return false;
		}

		outLoadData.resources.push_back({ key, std::move(navigationMesh) });
		return true;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadOutsideSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadEnvironmentResources(result, resources, resourceFactory, L"OutsideEnvironmentMap.bin") == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"jump_outside", L"jump_outside.bin") == false)
			return result;

		result.succeeded = true;
		return result;
	}

	CommonLoadingScene::SceneLoadData CommonLoadingScene::LoadQamilSceneResources(Resources& resources, IGraphicsResourceFactory& resourceFactory)
	{
		SceneLoadData result{};

		if (LoadMeshTextures(result, resources, resourceFactory) == false)
			return result;

		if (LoadEnvironmentResources(result, resources, resourceFactory, L"QamilEnvironmentMap.bin") == false)
			return result;

		if (LoadNavigationMesh(result, resources, L"qamil", L"qamil.bin") == false)
			return result;

		result.succeeded = true;
		return result;
	}

	bool CommonLoadingScene::RegisterLoadedResources()
	{
		Resources& resources = APPLICATION.GetResources();
		for (const ResourceLoadData& loadData : _sceneLoadData.resources)
		{
			GM_ASSERT_RETURN_VAL(
				resources.Add(loadData.key, loadData.resource),
				false,
				"로드한 리소스 등록에 실패했습니다. key=%ls",
				loadData.key.c_str());
		}

		return true;
	}

	void CommonLoadingScene::RequestTargetSceneTransition()
	{
		APPLICATION.GetSceneManager().RequestSceneChange(_pendingSceneName);
		_loadingState = LoadingState::TransitionRequested;
	}

	float CommonLoadingScene::GetLoadingProgress() const
	{
		if (_loadingState == LoadingState::ReadyToTransition ||
			_loadingState == LoadingState::TransitionRequested)
			return 1.f;

		return 0.f;
	}
}
