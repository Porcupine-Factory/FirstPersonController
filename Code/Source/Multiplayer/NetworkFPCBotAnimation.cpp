/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Multiplayer/NetworkFPCBotAnimation.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Integration/AnimGraphComponentBus.h>
#include <Integration/AnimGraphNetworkingBus.h>
#include <Integration/AnimationBus.h>

namespace FirstPersonController
{
    void NetworkFPCBotAnimation::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<NetworkFPCBotAnimation, NetworkFPCBotAnimationBase>()->Version(1);
        }
        NetworkFPCBotAnimationBase::Reflect(context);
    }

    NetworkFPCBotAnimation::NetworkFPCBotAnimation()
        : m_preRenderEventHandler(
              [this](float deltaTime)
              {
                  OnPreRender(deltaTime);
              })
        , m_enableNetworkAnimationChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkAnimationChanged(enable);
              })
    {
        ;
    }

    void NetworkFPCBotAnimation::OnInit()
    {
    }

    void NetworkFPCBotAnimation::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        // Get access to the FirstPersonControllerComponent object and its members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
        m_firstPersonControllerObject->m_isNetBot = true;

        // Subscribe to EnableNetworkFPCBotAnimation change events
        EnableNetworkAnimationAddEvent(m_enableNetworkAnimationChangedEvent);

        // Find child entity with anim graph component
        DetectAnimationChild();

        // Network animation setup
        if (!m_animationEntityId.IsValid())
        {
            AZLOG_WARN("No animation child detected. Falling back to parent entity (self).");
            m_animationEntityId = GetEntityId();
        }

        // Network animation setup
        SetupAnimationConnections(m_animationEntityId);

        if (GetEnableNetworkAnimation())
        {
            GetNetBindComponent()->AddEntityPreRenderEventHandler(m_preRenderEventHandler);
        }
    }

    void NetworkFPCBotAnimation::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetEnableNetworkAnimation())
        {
            EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
        }
        m_enableNetworkAnimationChangedEvent.Disconnect();
    }

    void NetworkFPCBotAnimation::OnEnableNetworkAnimationChanged(const bool& enable)
    {
        if (enable)
        {
            // Reconnect network animation buses and handler
            SetupAnimationConnections(m_animationEntityId);

            GetNetBindComponent()->AddEntityPreRenderEventHandler(m_preRenderEventHandler);
        }
        else
        {
            // Disconnect network animation buses and handler
            EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
            EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect();
            m_preRenderEventHandler.Disconnect();
            m_actorRequests = nullptr;
            m_networkRequests = nullptr;
            m_animationGraph = nullptr;
            m_paramIdsNotSet = true;
            m_walkSpeedParamId = BotInvalidParamIndex;
            m_sprintParamId = BotInvalidParamIndex;
            m_crouchToStandParamId = BotInvalidParamIndex;
            m_crouchParamId = BotInvalidParamIndex;
            m_standToCrouchParamId = BotInvalidParamIndex;
            m_jumpStartParamId = BotInvalidParamIndex;
            m_fallParamId = BotInvalidParamIndex;
            m_landParamId = BotInvalidParamIndex;
            m_groundedParamId = BotInvalidParamIndex;
        }
    }

    void NetworkFPCBotAnimation::DetectAnimationChild()
    {
        AZStd::vector<AZ::EntityId> children;
        AZ::TransformBus::EventResult(children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);

        for (const AZ::EntityId& childId : children)
        {
            if (EMotionFX::Integration::AnimGraphComponentRequestBus::FindFirstHandler(childId) != nullptr)
            {
                m_animationEntityId = childId;

                // auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
                // AZ_Printf(
                //     "Network FPC Component",
                //     "Animation child EntityId = %llu, Animation child entity name = %s",
                //     static_cast<AZ::u64>(m_animationEntityId),
                //     ca->FindEntity(m_animationEntityId)->GetName().c_str());

                SetupAnimationConnections(m_animationEntityId);

                m_animationChildFound = true;
                return;
            }
        }
    }

    void NetworkFPCBotAnimation::SetupAnimationConnections(const AZ::EntityId& targetId)
    {
        if (!GetEnableNetworkAnimation() || !targetId.IsValid())
        {
            return;
        }

        // If changing IDs, disconnect from old
        if (targetId != m_animationEntityId && m_animationEntityId.IsValid())
        {
            EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect(m_animationEntityId);
            EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect(m_animationEntityId);
        }

        m_animationEntityId = targetId;
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(m_animationEntityId);
        m_networkRequests = EMotionFX::AnimGraphComponentNetworkRequestBus::FindFirstHandler(m_animationEntityId);
        m_animationGraph = EMotionFX::Integration::AnimGraphComponentRequestBus::FindFirstHandler(m_animationEntityId);

        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(m_animationEntityId);
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusConnect(m_animationEntityId);
    }

    void NetworkFPCBotAnimation::OnPreRender(float deltaTime)
    {
        if (!m_animationChildFound)
        {
            DetectAnimationChild();
        }

        if (!GetEnableNetworkAnimation() || m_animationGraph == nullptr || m_networkRequests == nullptr)
        {
            return;
        }

        if (m_paramIdsNotSet)
        {
            m_paramIdsNotSet = false;
            if (m_walkSpeedParamId == InvalidParamIndex)
                m_walkSpeedParamId = m_animationGraph->FindParameterIndex(GetWalkSpeedParamName().c_str());
            if (m_sprintParamId == InvalidParamIndex)
                m_sprintParamId = m_animationGraph->FindParameterIndex(GetSprintParamName().c_str());
            if (m_standToCrouchParamId == InvalidParamIndex)
                m_standToCrouchParamId = m_animationGraph->FindParameterIndex(GetStandToCrouchParamName().c_str());
            if (m_crouchParamId == InvalidParamIndex)
                m_crouchParamId = m_animationGraph->FindParameterIndex(GetCrouchParamName().c_str());
            if (m_crouchToStandParamId == InvalidParamIndex)
                m_crouchToStandParamId = m_animationGraph->FindParameterIndex(GetCrouchToStandParamName().c_str());
            if (m_jumpStartParamId == InvalidParamIndex)
                m_jumpStartParamId = m_animationGraph->FindParameterIndex(GetJumpStartParamName().c_str());
            if (m_fallParamId == InvalidParamIndex)
                m_fallParamId = m_animationGraph->FindParameterIndex(GetFallParamName().c_str());
            if (m_landParamId == InvalidParamIndex)
                m_landParamId = m_animationGraph->FindParameterIndex(GetLandParamName().c_str());
            if (m_groundedParamId == InvalidParamIndex)
                m_groundedParamId = m_animationGraph->FindParameterIndex(GetGroundedParamName().c_str());
        }

        // Get networked velocity from component base
        if (m_walkSpeedParamId != BotInvalidParamIndex)
        {
            AZ::Vector2 velocity = m_firstPersonControllerObject->m_applyVelocityXY;
            float speed = velocity.GetLength();

            // Set the parameter directly; anim graph handles transitions
            m_animationGraph->SetParameterFloat(m_walkSpeedParamId, speed);
        }

        if (m_sprintParamId != BotInvalidParamIndex)
        {
            const bool isSprinting = GetIsSprinting();
            m_animationGraph->SetParameterBool(m_sprintParamId, isSprinting);
        }

        if (m_standToCrouchParamId != BotInvalidParamIndex)
        {
            const bool isCrouchingDownMove = GetIsCrouchingDownMove();
            m_animationGraph->SetParameterBool(m_standToCrouchParamId, isCrouchingDownMove);
        }

        if (m_crouchToStandParamId != BotInvalidParamIndex)
        {
            const bool isStandingUpMove = GetIsStandingUpMove();
            m_animationGraph->SetParameterBool(m_crouchToStandParamId, isStandingUpMove);
        }

        if (m_crouchParamId != BotInvalidParamIndex)
        {
            const bool isCrouching = GetIsCrouching();
            m_animationGraph->SetParameterBool(m_crouchParamId, isCrouching);
        }

        if (m_jumpStartParamId != BotInvalidParamIndex)
        {
            const bool isJumpStarting = GetIsJumpStarting();
            m_animationGraph->SetParameterBool(m_jumpStartParamId, isJumpStarting);
        }

        if (m_fallParamId != BotInvalidParamIndex)
        {
            const bool isFalling = GetIsFalling();
            m_animationGraph->SetParameterBool(m_fallParamId, isFalling);
        }

        if (m_landParamId != BotInvalidParamIndex)
        {
            const bool isLanding = GetIsLanding();
            m_animationGraph->SetParameterBool(m_landParamId, isLanding);
        }

        if (m_groundedParamId != BotInvalidParamIndex)
        {
            const bool isGrounded = GetIsGrounded();
            m_animationGraph->SetParameterBool(m_groundedParamId, isGrounded);
        }

        m_networkRequests->UpdateActorExternal(deltaTime);
    }

    void NetworkFPCBotAnimation::OnActorInstanceCreated([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(m_animationEntityId);
    }

    void NetworkFPCBotAnimation::OnActorInstanceDestroyed([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = nullptr;
    }

    void NetworkFPCBotAnimation::OnAnimGraphInstanceCreated([[maybe_unused]] EMotionFX::AnimGraphInstance* animGraphInstance)
    {
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect();

        if (m_actorRequests)
        {
            // Network controls transforms
            m_actorRequests->EnableInstanceUpdate(false);
        }
    }

    NetworkFPCBotAnimationController::NetworkFPCBotAnimationController(NetworkFPCBotAnimation& parent)
        : NetworkFPCBotAnimationControllerBase(parent)
    {
    }

    void NetworkFPCBotAnimationController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkFPCBotAnimationController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkFPCBotAnimationController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void NetworkFPCBotAnimationController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkFPCBotAnimationController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkFPCBotAnimationService"));
    }

    void NetworkFPCBotAnimationController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkFPCBotAnimationService"));
    }

    void NetworkFPCBotAnimationController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkFPCBotAnimationController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
    }
} // namespace FirstPersonController
