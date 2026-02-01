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
            serializeContext->Class<NetworkFPCBotAnimation, NetworkFPCBotAnimationBase>()->Version(1);

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
            m_animationEntityId = GetEntityId();

        // Network animation setup
        SetupAnimationConnections(m_animationEntityId);

        if (GetEnableNetworkAnimation())
            GetNetBindComponent()->AddEntityPreRenderEventHandler(m_preRenderEventHandler);
    }

    void NetworkFPCBotAnimation::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetEnableNetworkAnimation())
            EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();

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
            m_paramIdsSet = false;
            m_walkSpeedParamId = InvalidParamIndex;
            m_sprintParamId = InvalidParamIndex;
            m_crouchToStandParamId = InvalidParamIndex;
            m_crouchParamId = InvalidParamIndex;
            m_standToCrouchParamId = InvalidParamIndex;
            m_jumpStartParamId = InvalidParamIndex;
            m_fallParamId = InvalidParamIndex;
            m_landParamId = InvalidParamIndex;
            m_groundedParamId = InvalidParamIndex;
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
            return;

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
            DetectAnimationChild();

        if (!GetEnableNetworkAnimation() || m_animationGraph == nullptr || m_networkRequests == nullptr)
            return;

        // Set the para Ids if they're not all currently set
        if (!m_paramIdsSet)
        {
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

            m_paramIdsSet = true;
            for (size_t* paramId : m_paramIds)
                if (*paramId == InvalidParamIndex)
                    m_paramIdsSet = false;
        }

        // Set the animation graph values
        if (m_paramIdsSet)
        {
            m_animationGraph->SetParameterFloat(m_walkSpeedParamId, GetApplyVelocityXY().GetLength());
            m_animationGraph->SetParameterBool(m_sprintParamId, GetIsSprinting());
            m_animationGraph->SetParameterBool(m_standToCrouchParamId, GetIsCrouchingDownMove());
            m_animationGraph->SetParameterBool(m_crouchToStandParamId, GetIsStandingUpMove());
            m_animationGraph->SetParameterBool(m_crouchParamId, GetIsCrouching());
            m_animationGraph->SetParameterBool(m_jumpStartParamId, GetIsJumpStarting());
            m_animationGraph->SetParameterBool(m_fallParamId, GetIsFalling());
            m_animationGraph->SetParameterBool(m_landParamId, GetIsLanding());
            m_animationGraph->SetParameterBool(m_groundedParamId, GetIsGrounded());
        }
        else
        {
            if (m_walkSpeedParamId != InvalidParamIndex)
                m_animationGraph->SetParameterFloat(m_walkSpeedParamId, GetApplyVelocityXY().GetLength());
            if (m_sprintParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_sprintParamId, GetIsSprinting());
            if (m_standToCrouchParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_standToCrouchParamId, GetIsCrouchingDownMove());
            if (m_crouchToStandParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_crouchToStandParamId, GetIsStandingUpMove());
            if (m_crouchParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_crouchParamId, GetIsCrouching());
            if (m_jumpStartParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_jumpStartParamId, GetIsJumpStarting());
            if (m_fallParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_fallParamId, GetIsFalling());
            if (m_landParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_landParamId, GetIsLanding());
            if (m_groundedParamId != InvalidParamIndex)
                m_animationGraph->SetParameterBool(m_groundedParamId, GetIsGrounded());
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
            m_actorRequests->EnableInstanceUpdate(false);
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

    void NetworkFPCBotAnimationController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
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

    void NetworkFPCBotAnimationController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }
} // namespace FirstPersonController
