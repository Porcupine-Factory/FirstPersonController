#include <Multiplayer/NetworkFPC.h>
#include <Multiplayer/NetworkFPCAnimationComponent.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Integration/AnimGraphComponentBus.h>
#include <Integration/AnimGraphNetworkingBus.h>
#include <Integration/AnimationBus.h>

namespace FirstPersonController
{
    void NetworkFPCAnimationComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkFPCAnimationComponent, NetworkFPCAnimationComponentBase>()->Version(1);
        }
        NetworkFPCAnimationComponentBase::Reflect(context);
    }

    NetworkFPCAnimationComponent::NetworkFPCAnimationComponent()
        : m_preRenderEventHandler(
              [this](float deltaTime)
              {
                  OnPreRender(deltaTime);
              })
    {
        ;
    }

    void NetworkFPCAnimationComponent::OnInit()
    {
    }

    void NetworkFPCAnimationComponent::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(GetEntityId());
        m_networkRequests = EMotionFX::AnimGraphComponentNetworkRequestBus::FindFirstHandler(GetEntityId());
        m_animationGraph = EMotionFX::Integration::AnimGraphComponentRequestBus::FindFirstHandler(GetEntityId());

        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(GetEntityId());
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusConnect(GetEntityId());

        GetNetBindComponent()->AddEntityPreRenderEventHandler(m_preRenderEventHandler);
    }

    void NetworkFPCAnimationComponent::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
    }

    void NetworkFPCAnimationComponent::OnPreRender([[maybe_unused]] float deltaTime)
    {
        if (m_animationGraph == nullptr || m_networkRequests == nullptr)
        {
            return;
        }

        if (m_walkSpeedParamId == InvalidParamIndex)
        {
            m_walkSpeedParamId = m_animationGraph->FindParameterIndex(GetWalkSpeedParamName().c_str());
        }

        // Get networked velocity from controller
        NetworkFPC* controller = GetEntity()->FindComponent<NetworkFPC>();
        if (controller && m_walkSpeedParamId != InvalidParamIndex)
        {
            AZ::Vector3 velocity = controller->GetDesiredVelocity();
            // Ignore Z for ground speed
            velocity.SetZ(0.0f);
            const float speed = velocity.GetLength();

            // Set the paramater directly; anim graph handles transitions
            m_animationGraph->SetParameterFloat(m_walkSpeedParamId, speed);
        }

        m_networkRequests->UpdateActorExternal(deltaTime);
    }

    void NetworkFPCAnimationComponent::OnActorInstanceCreated([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(GetEntityId());
    }

    void NetworkFPCAnimationComponent::OnActorInstanceDestroyed([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = nullptr;
    }

    void NetworkFPCAnimationComponent::OnAnimGraphInstanceCreated([[maybe_unused]] EMotionFX::AnimGraphInstance* animGraphInstance)
    {
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect();

        if (m_actorRequests)
        {
            // Network controls transforms
            m_actorRequests->EnableInstanceUpdate(false);
        }
    }
} // namespace FirstPersonController
