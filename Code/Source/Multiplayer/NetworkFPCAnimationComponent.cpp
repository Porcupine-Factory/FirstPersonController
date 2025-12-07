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

        if (m_sprintParamId == InvalidParamIndex)
        {
            m_sprintParamId = m_animationGraph->FindParameterIndex(GetSprintParamName().c_str());
        }

        if (m_standToCrouchParamId == InvalidParamIndex)
        {
            m_standToCrouchParamId = m_animationGraph->FindParameterIndex(GetStandToCrouchParamName().c_str());
        }

        if (m_crouchParamId == InvalidParamIndex)
        {
            m_crouchParamId = m_animationGraph->FindParameterIndex(GetCrouchParamName().c_str());
        }

        if (m_crouchToStandParamId == InvalidParamIndex)
        {
            m_crouchToStandParamId = m_animationGraph->FindParameterIndex(GetCrouchToStandParamName().c_str());
        }

        if (m_jumpStartParamId == InvalidParamIndex)
        {
            m_jumpStartParamId = m_animationGraph->FindParameterIndex(GetJumpStartParamName().c_str());
        }

        if (m_fallParamId == InvalidParamIndex)
        {
            m_fallParamId = m_animationGraph->FindParameterIndex(GetFallParamName().c_str());
        }

        if (m_jumpLandParamId == InvalidParamIndex)
        {
            m_jumpLandParamId = m_animationGraph->FindParameterIndex(GetJumpLandParamName().c_str());
        }

        if (m_groundedParamId == InvalidParamIndex)
        {
            m_groundedParamId = m_animationGraph->FindParameterIndex(GetGroundedParamName().c_str());
        }

        // Get networked velocity from controller
        NetworkFPC* controller = GetEntity()->FindComponent<NetworkFPC>();
        if (controller)
        {
            if (m_walkSpeedParamId != InvalidParamIndex)
            {
                AZ::Vector3 velocity = controller->GetDesiredVelocity();
                // Ignore Z for ground speed
                velocity.SetZ(0.0f);
                float speed = velocity.GetLength();

                // Set the paramater directly; anim graph handles transitions
                m_animationGraph->SetParameterFloat(m_walkSpeedParamId, speed);
            }

            if (m_sprintParamId != InvalidParamIndex)
            {
                bool isSprinting = controller->GetIsSprinting();
                m_animationGraph->SetParameterBool(m_sprintParamId, isSprinting);
            }

            if (m_standToCrouchParamId != InvalidParamIndex)
            {
                bool isCrouchingDown = controller->GetIsCrouchingDown();
                m_animationGraph->SetParameterBool(m_standToCrouchParamId, isCrouchingDown);
            }

            if (m_crouchToStandParamId != InvalidParamIndex)
            {
                bool isStandingUp = controller->GetIsStandingUp();
                m_animationGraph->SetParameterBool(m_crouchToStandParamId, isStandingUp);
            }

            if (m_crouchParamId != InvalidParamIndex)
            {
                bool isCrouching = controller->GetIsCrouching();
                m_animationGraph->SetParameterBool(m_crouchParamId, isCrouching);
            }

            if (m_jumpStartParamId != InvalidParamIndex)
            {
                bool isJumpStarting = controller->GetIsJumpStarting();
                m_animationGraph->SetParameterBool(m_jumpStartParamId, isJumpStarting);
            }

            if (m_fallParamId != InvalidParamIndex)
            {
                bool isFalling = controller->GetIsFalling();
                m_animationGraph->SetParameterBool(m_fallParamId, isFalling);
            }

            if (m_jumpLandParamId != InvalidParamIndex)
            {
                bool isJumpLanding = controller->GetIsJumpLanding();
                m_animationGraph->SetParameterBool(m_jumpLandParamId, isJumpLanding);
            }

            if (m_groundedParamId != InvalidParamIndex)
            {
                bool isGrounded = controller->GetIsGrounded();
                m_animationGraph->SetParameterBool(m_groundedParamId, isGrounded);
            }
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
