#include "DebugEventPublisher.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "DebugInputHandler.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"
#include "IDebugRenderer.h"
#include "MathUtil.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "TemporaryBoxHitBoxObject.h"
#include "TransformComponent.h"
#include "UIManager.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t DebugInputType[] = L"HiFiRush.DebugEvent";
		constexpr float DebugAttackDuration = 0.15f;
		constexpr float DebugAttackForwardOffset = 2.f;

		void SpawnPlayerDebugAttack()
		{
			GameplayScene* scene = dynamic_cast<GameplayScene*>(APPLICATION.GetSceneManager().GetActiveScene());
			if (scene == nullptr)
				return;

			GameObject* player = scene->GetPlayer().Get();
			if (player == nullptr || player->GetTransform() == nullptr)
				return;

			const TransformComponent& playerTransform = *player->GetTransform();
			const Vector3 forward = Math::GetNormalizedXZDirection(Math::GetLookVector(playerTransform.GetRotation()));
			TemporaryBoxHitBoxDesc desc{};
			desc.world = Math::CreateTransformMatrix(playerTransform.GetPosition() + forward * DebugAttackForwardOffset, playerTransform.GetRotation());
			desc.localCenter = Vector3{ 0.f, 0.9f, 0.f };
			desc.size = Vector3{ 2.f, 1.8f, 2.f };
			desc.collisionLayer = HiFiRushCollisionLayer::PlayerAttack;
			desc.collisionMask = HiFiRushCollisionLayer::Monster;
			desc.damageInfo.amount = 10;
			desc.lifetime = DebugAttackDuration;
			scene->SpawnGameObject<TemporaryHitBoxObject>(desc);
		}
	}

	void DebugEventPublisher::Initialize()
	{
		DebugInputHandler::RegisterDebugType(DebugInputType, true);
	}

	void DebugEventPublisher::Tick()
	{
		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F4))
		{
			DebugEvent event{};
			event.type = DebugEventType::Activate;
			OnDebugEvent.Publish(event);
		}
		else if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F9))
		{
			DebugEvent event{};
			event.type = DebugEventType::Reset;
			OnDebugEvent.Publish(event);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F11))
		{
			IDebugRenderer& debugRenderer = APPLICATION.GetDebugRenderer();
			const bool isEnabled = debugRenderer.IsEnabled() == false;
			debugRenderer.SetEnabled(isEnabled);
			APPLICATION.GetUIManager().SetDebugWidgetsVisible(isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::B))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsFrustumCullingEnabled() == false;
				renderer.SetFrustumCullingEnabled(isEnabled);
			}
			else
			{
				const bool isEnabled = renderer.IsBoundingVolumeDebugDrawEnabled() == false;
				renderer.SetBoundingVolumeDebugDrawEnabled(isEnabled);
			}
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::I))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsStaticMeshInstancingEnabled() == false;
				renderer.SetStaticMeshInstancingEnabled(isEnabled);
			}
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::C))
		{
			PhysicsSystem& physicsSystem = APPLICATION.GetPhysicsSystem();
			const bool isEnabled = physicsSystem.IsCollider3DDebugDrawEnabled();
			physicsSystem.SetCollider3DDebugDrawEnabled(!isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::N))
		{
			NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
			const bool isEnabled = navMeshSystem.IsDebugDrawEnabled();
			navMeshSystem.SetDebugDrawEnabled(!isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::H))
			SpawnPlayerDebugAttack();
	}
}

#endif
