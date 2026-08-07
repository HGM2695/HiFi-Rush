#include "QamilScene.h"
#include "Application.h"
#include "CameraManager.h"
#include "EnvironmentSpawner.h"
#include "HiFiRushAudio.h"
#include "Input.h"
#include "MathUtil.h"
#include "MapResource.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "PlayerSpawner.h"
#include "Resources.h"
#include "SceneDebugTools.h"

namespace gm
{
	void QamilScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);

		const std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"qamil");
		GM_ASSERT_RETURN(navigationMesh, "qamil NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);

		APPLICATION.GetInput().SetCursorLocked(true);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Qamil);
	}

	void QamilScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void QamilScene::OnInitialize()
	{
		InitializeEnvironment();

		PlayerSpawner playerSpawner(APPLICATION.GetResources());
		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ -0.43f, 0.f, -6.13f };
		playerDesc.cameraDistance = 9.f;
		playerDesc.cameraYaw = Math::DegreesToRadians(-90.f);
		playerDesc.cameraPitch = Math::DegreesToRadians(10.f);
		playerDesc.cameraHeight = 3.f;
		GM_ASSERT_RETURN(playerSpawner.Spawn(*this, playerDesc), "Qamil Player 생성에 실패했습니다.");
	}

	void QamilScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
	}

	void QamilScene::InitializeEnvironment()
	{
		const std::shared_ptr<MapResource> mapResource = APPLICATION.GetResources().Find<MapResource>(L"QamilMap");
		GM_ASSERT_RETURN(mapResource, "Qamil MapResource가 로드되지 않았습니다.");

		EnvironmentSpawner spawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN(spawner.Spawn(*this, mapResource->GetData()), "Qamil 환경 구성에 실패했습니다.");
	}
}
