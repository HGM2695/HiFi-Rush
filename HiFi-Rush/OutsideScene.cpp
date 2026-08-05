#include "OutsideScene.h"
#include "Application.h"
#include "BinaryEnvironmentMapLoader.h"
#include "CameraManager.h"
#include "EnvironmentMapTypes.h"
#include "EnvironmentSpawner.h"
#include "HiFiRushAudio.h"
#include "Input.h"
#include "MathUtil.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "Paths.h"
#include "PhysicsSystem.h"
#include "PlayerSpawner.h"
#include "Resources.h"
#include "SceneDebugTools.h"

namespace gm
{
	void OutsideScene::OnEnter()
	{
		APPLICATION.GetPhysicsSystem().SetPhysicsMode(PhysicsMode::Physics3D);

		const std::shared_ptr<NavigationMesh> navigationMesh = APPLICATION.GetResources().Find<NavigationMesh>(L"jump_outside");
		GM_ASSERT_RETURN(navigationMesh, "jump_outside NavigationMesh가 로드되지 않았습니다.");
		APPLICATION.GetPhysicsSystem().GetNavMeshSystem().SetActiveNavigationMesh(navigationMesh);

		APPLICATION.GetInput().SetCursorLocked(true);
		GetCameraManager()->SetActiveCamera(PlayerCameraKey);
		PlayRhythmBGM(HiFiRushBGM::Outside);
	}

	void OutsideScene::OnExit()
	{
		APPLICATION.GetInput().SetCursorLocked(false);
	}

	void OutsideScene::OnInitialize()
	{
		InitializeEnvironment();

		PlayerSpawner playerSpawner(APPLICATION.GetResources());
		PlayerSpawnDesc playerDesc{};
		playerDesc.position = Vector3{ -0.1f, 0.f, 11.f };
		playerDesc.rotationY = Math::GM_PI;
		playerDesc.cameraDistance = 5.f;
		playerDesc.cameraYaw = Math::DegreesToRadians(90.f);
		playerDesc.cameraPitch = Math::DegreesToRadians(20.f);
		GM_ASSERT_RETURN(playerSpawner.Spawn(*this, playerDesc), "Outside Player 생성에 실패했습니다.");
	}

	void OutsideScene::OnTick(float deltaTime)
	{
		TickSceneTransitionDebug();
	}

	void OutsideScene::InitializeEnvironment()
	{
		EnvironmentMapData mapData{};
		GM_ASSERT_RETURN(BinaryEnvironmentMapLoader::Load(GetMapPath(L"OutsideEnvironmentMap.bin"), mapData), "Outside 환경 맵을 로드하지 못했습니다.");

		EnvironmentSpawner spawner(APPLICATION.GetResources());
		GM_ASSERT_RETURN(spawner.Spawn(*this, mapData), "Outside 환경 오브젝트 생성에 실패했습니다.");
	}
}
