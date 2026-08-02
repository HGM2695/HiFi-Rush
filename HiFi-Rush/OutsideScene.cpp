#include "OutsideScene.h"
#include "Application.h"
#include "BinaryEnvironmentMapLoader.h"
#include "EnvironmentMapTypes.h"
#include "EnvironmentSpawner.h"
#include "NavigationMesh.h"
#include "NavMeshSystem.h"
#include "Paths.h"
#include "PhysicsSystem.h"
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

		ActivateEnvironmentOverviewCamera(*this);
	}

	void OutsideScene::OnInitialize()
	{
		InitializeEnvironment();
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
		CreateEnvironmentOverviewCamera(*this, Vector3{ -0.1f, 4.f, 18.f }, Vector3{ -0.1f, 1.f, 5.f });
	}
}
