#include "DebugEventPublisher.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "BoxCollider3DComponent.h"
#include "DebugInputHandler.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"
#include "HitBoxComponent.h"
#include "IDebugRenderer.h"
#include "MathUtil.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "TransformComponent.h"
#include "UIManager.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t DebugInputType[] = L"HiFiRush.DebugEvent";
		constexpr float DebugAttackDuration = 0.15f;
		constexpr float DebugAttackForwardOffset = 2.f;

		class DebugAttackHitBoxObject final : public GameObject
		{
		public:
			explicit DebugAttackHitBoxObject(const Vector3& position) : GameObject(position) {}

			void SetHitBox(HitBoxComponent& hitBox) { _hitBox = &hitBox; }

		protected:
			void OnTick(float deltaTime) override
			{
				if (_hitBox == nullptr)
				{
					Destroy();
					return;
				}

				if (_hasStarted == false)
				{
					_hitBox->BeginAttack();
					_hasStarted = true;
				}

				_remainingTime -= deltaTime;
				if (_remainingTime <= 0.f)
					Destroy();
			}

		private:
			HitBoxComponent*	_hitBox = nullptr;
			float				_remainingTime = DebugAttackDuration;
			bool				_hasStarted = false;
		};

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
			DebugAttackHitBoxObject* attackObject = scene->SpawnGameObject<DebugAttackHitBoxObject>(playerTransform.GetPosition() + forward * DebugAttackForwardOffset);
			attackObject->GetTransform()->SetRotation(playerTransform.GetRotation());

			BoxCollider3DComponent* collider = attackObject->AddComponent<BoxCollider3DComponent>();
			collider->SetLocalCenter(Vector3{ 0.f, 0.9f, 0.f });
			collider->SetSize(Vector3{ 2.f, 1.8f, 2.f });
			collider->SetCollisionLayer(HiFiRushCollisionLayer::PlayerAttack);
			collider->SetCollisionMask(HiFiRushCollisionLayer::Monster);

			HitBoxComponent* hitBox = attackObject->AddComponent<HitBoxComponent>(*collider);
			hitBox->SetDamage(10);
			attackObject->SetHitBox(*hitBox);
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
