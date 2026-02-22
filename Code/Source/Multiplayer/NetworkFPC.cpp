/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Multiplayer/Components/NetworkCharacterComponent.h>

#include <AzNetworking/Framework/INetworking.h>

#include <Integration/AnimGraphComponentBus.h>
#include <Integration/AnimGraphNetworkingBus.h>
#include <Integration/AnimationBus.h>

namespace FirstPersonController
{
    AZ_CVAR(
        float,
        cl_VelocityTolerance,
        50.0f,
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "The tolerance used for ground obstruction checks, set this to a large number to avoid false-positive checks");

    using namespace StartingPointInput;

    void NetworkFPC::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
            serializeContext->Class<NetworkFPC, NetworkFPCBase>()->Version(1);

        NetworkFPCBase::Reflect(context);
    }

    NetworkFPC::NetworkFPC()
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

    void NetworkFPC::OnInit()
    {
    }

    void NetworkFPC::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        // Get access to the FirstPersonControllerComponent and FirstPersonExtrasComponent objects and their members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();

        // Subscribe to EnableNetworkFPC change events
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

    void NetworkFPC::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (GetEnableNetworkAnimation())
            EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();

        m_enableNetworkAnimationChangedEvent.Disconnect();
    }

    void NetworkFPC::OnEnableNetworkAnimationChanged(const bool& enable)
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

    void NetworkFPC::DetectAnimationChild()
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

    void NetworkFPC::SetupAnimationConnections(const AZ::EntityId& targetId)
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

    void NetworkFPC::OnPreRender(float deltaTime)
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

    void NetworkFPC::OnActorInstanceCreated([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = EMotionFX::Integration::ActorComponentRequestBus::FindFirstHandler(m_animationEntityId);
    }

    void NetworkFPC::OnActorInstanceDestroyed([[maybe_unused]] EMotionFX::ActorInstance* actorInstance)
    {
        m_actorRequests = nullptr;
    }

    void NetworkFPC::OnAnimGraphInstanceCreated([[maybe_unused]] EMotionFX::AnimGraphInstance* animGraphInstance)
    {
        EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler::BusDisconnect();

        if (m_actorRequests)
            m_actorRequests->EnableInstanceUpdate(false);
    }

    NetworkFPCController::NetworkFPCController(NetworkFPC& parent)
        : NetworkFPCControllerBase(parent)
        , m_enableNetworkFPCChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkFPCChanged(enable);
              })
        , m_playerStringNetEntityIdsChangedEvent(
              [this](AZStd::vector<AZStd::string> playerStringEntityIds)
              {
                  OnPlayerStringNetEntityIdsChanged(playerStringEntityIds);
              })
        , m_botStringNetEntityIdsChangedEvent(
              [this](AZStd::vector<AZStd::string> botStringEntityIds)
              {
                  OnBotStringNetEntityIdsChanged(botStringEntityIds);
              })
    {
    }

    void NetworkFPCController::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if (m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("NetworkFPC Controller", false, "Number of input IDs not equal to number of input names!");
        }
        else
        {
            const AZ::u8 size = sizeof(m_inputNames) / sizeof(AZStd::string*);

            for (AZ::u8 i = 0; i < size; ++i)
                m_inputNames[i] = m_firstPersonControllerObject->m_inputNames[i];

            for (auto& it_event : m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
    }

    void NetworkFPCController::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Network FPC Component", "Pressed: %s", it_event.first->ToString().c_str());
            }
        }
    }

    void NetworkFPCController::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Network FPC Component", "Released: %s", it_event.first->ToString().c_str());
            }
        }
    }

    void NetworkFPCController::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        if (*inputId == m_rotateYawEventId)
        {
            m_yawValue += value;
        }
        else if (*inputId == m_rotatePitchEventId)
        {
            m_pitchValue += value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if (*inputId == m_sprintEventId)
        {
            const bool notRecentlyGrounded = AZStd::all_of(
                m_firstPersonControllerObject->m_prevNTicksGrounded.begin(),
                m_firstPersonControllerObject->m_prevNTicksGrounded.end(),
                [](bool gnd)
                {
                    return gnd == false;
                });
            m_groundedRecently = !notRecentlyGrounded;

            if (m_firstPersonControllerObject->m_grounded || m_groundedRecently ||
                m_firstPersonControllerObject->m_sprintPrevValue == 0.f || m_firstPersonControllerObject->m_sprintInAir)
                m_sprintValue = value;
            else
                m_sprintValue = 0.f;
        }
    }

    void NetworkFPCController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkFPCControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to network property change events
        EnableNetworkFPCAddEvent(m_enableNetworkFPCChangedEvent);
        PlayerStringNetEntityIdsAddEvent(m_playerStringNetEntityIdsChangedEvent);
        BotStringNetEntityIdsAddEvent(m_botStringNetEntityIdsChangedEvent);

        // Get access to the FirstPersonControllerComponent and FirstPersonExtrasComponent objects and their members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
        m_firstPersonExtrasObject = entity->FindComponent<FirstPersonExtrasComponent>();
        m_firstPersonControllerObject->m_networkFPCEnabled = GetEnableNetworkFPC();
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = GetEnableNetworkFPC();

        // Set the velocity tolerance to a big number to avoid false-positive ground obstruction checks
        m_firstPersonControllerObject->m_velocityCloseTolerance = cl_VelocityTolerance;

        m_firstPersonControllerObject->IgnoreInputs(true);
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->IgnoreInputs(true);

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_firstPersonControllerObject->IsAutonomousSoConnect();
            m_firstPersonControllerObject->m_isNetBot = false;
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->IsAutonomousSoConnect();
            AssignConnectInputEvents();
            if (IsNetEntityRoleAuthority())
            {
                m_firstPersonControllerObject->m_isHost = true;
                NetworkFPCControllerNotificationBus::Broadcast(
                    &NetworkFPCControllerNotificationBus::Events::OnHostActivated, GetEntityId());
            }
            else
            {
                m_firstPersonControllerObject->m_isAutonomousClient = true;
                NetworkFPCControllerNotificationBus::Broadcast(
                    &NetworkFPCControllerNotificationBus::Events::OnAutonomousClientActivated, GetEntityId());
            }

            m_firstPersonControllerObject->m_eyeHeight = GetEyeHeight();
        }
        else
            NetworkFPCControllerNotificationBus::Broadcast(
                &NetworkFPCControllerNotificationBus::Events::OnNonAutonomousClientActivated, GetEntityId());

#if AZ_TRAIT_SERVER
        if (IsNetEntityRoleAuthority())
        {
            AZ::Interface<Multiplayer::IMultiplayer>::Get()->AddConnectionAcquiredHandler(m_connectionAcquiredHandler);
            AZ::Interface<Multiplayer::IMultiplayer>::Get()->AddEndpointDisconnectedHandler(m_endpointDisconnectedHandler);
        }
#endif
    }

    void NetworkFPCController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkFPCControllerRequestBus::Handler::BusDisconnect();
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        m_enableNetworkFPCChangedEvent.Disconnect();
        m_playerStringNetEntityIdsChangedEvent.Disconnect();
        m_botStringNetEntityIdsChangedEvent.Disconnect();
        m_connectionAcquiredHandler.Disconnect();
        m_endpointDisconnectedHandler.Disconnect();
    }

    void NetworkFPCController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void NetworkFPCController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("InputConfigurationService"));
        dependent.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
    }

    void NetworkFPCController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkFPCService"));
    }

    void NetworkFPCController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkFPCService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void NetworkFPCController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        if (m_disabled)
            return;

        auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        // Assign input values
        if (m_allowAllMovementInputs)
        {
            if (m_allowRotationInputs)
            {
                playerInput->m_pitch = m_pitchValue;
                playerInput->m_yaw = m_yawValue;
            }
            playerInput->m_forward = m_forwardValue;
            playerInput->m_back = m_backValue;
            playerInput->m_left = m_leftValue;
            playerInput->m_right = m_rightValue;
            playerInput->m_sprint = m_sprintValue;
            playerInput->m_crouch = m_crouchValue;
            playerInput->m_jump = m_jumpValue;
        }

        playerInput->m_desiredVelocity = GetDesiredVelocity();
        playerInput->m_yawDelta = GetLookRotationDelta().GetZ();
        playerInput->m_yawDeltaOvershoot = GetYawDeltaOvershoot();
        playerInput->m_overrideTransformForTick = GetOverrideTransformForTick();
        playerInput->m_overrideRotationForTick = GetOverrideRotationForTick();
        playerInput->m_overrideTransform = GetOverrideTransform();

        m_yawValue = 0.0f;
        m_pitchValue = 0.0f;
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        // Initialize various network properties to the initial values in the First Person Controller component
        if (m_init)
        {
            m_init = false;
            SetTopWalkSpeed(m_firstPersonControllerObject->m_speed);
            SetSprintMaxTime(m_firstPersonControllerObject->m_sprintMaxTime);
            SetSprintCooldownTime(m_firstPersonControllerObject->m_sprintTotalCooldownTime);
            SetJumpInitialVelocity(m_firstPersonControllerObject->m_jumpInitialVelocity);
        }

        // Disconnect from various buses when the NetworkFPCController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            if (!GetIsNetBot())
            {
                m_firstPersonControllerObject->m_isNetBot = false;
                m_firstPersonControllerObject->NotAutonomousSoDisconnect();
                if (m_firstPersonExtrasObject != nullptr)
                    m_firstPersonExtrasObject->NotAutonomousSoDisconnect();
            }
            else
                m_firstPersonControllerObject->m_isNetBot = true;
            if (IsNetEntityRoleAuthority())
            {
                m_firstPersonControllerObject->m_isServer = true;
                // Set the server's FPC EntityId to be obtained by the autonomous client
                SetServerFPCEntityId(AZ::u64(m_firstPersonControllerObject->GetEntity()));
                FirstPersonControllerComponent::GetPlayerStringNetEntityIdsOnServer();
                SetPlayerStringNetEntityIds(FirstPersonControllerComponent::m_playerStringNetEntityIds);
                FirstPersonControllerComponent::m_reacquirePlayerBotStringNetEntityIds = true;
            }
            if (!m_firstPersonControllerObject->m_isServer && !m_firstPersonControllerObject->m_isNetBot)
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        const auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        // Assign the First Person Controller's inputs from the network inputs
        m_firstPersonControllerObject->m_forwardValue = playerInput->m_forward;
        m_firstPersonControllerObject->m_backValue = playerInput->m_back;
        m_firstPersonControllerObject->m_leftValue = playerInput->m_left;
        m_firstPersonControllerObject->m_rightValue = playerInput->m_right;
        m_firstPersonControllerObject->m_yawValue = playerInput->m_yaw;
        m_firstPersonControllerObject->m_pitchValue = playerInput->m_pitch;
        m_firstPersonControllerObject->m_sprintValue = playerInput->m_sprint;
        m_firstPersonControllerObject->m_crouchValue = playerInput->m_crouch;
        m_firstPersonControllerObject->m_jumpValue = playerInput->m_jump;

        if (playerInput->m_sprint != 0.f &&
            (m_firstPersonControllerObject->m_grounded || m_groundedRecently || m_firstPersonControllerObject->m_sprintPrevValue == 0.f ||
             m_firstPersonControllerObject->m_sprintInAir))
        {
            m_firstPersonControllerObject->m_sprintEffectiveValue = playerInput->m_sprint;
            m_firstPersonControllerObject->m_sprintAccelValue = playerInput->m_sprint * m_firstPersonControllerObject->m_sprintAccelScale;
        }
        else
        {
            m_firstPersonControllerObject->m_sprintEffectiveValue = 0.f;
            m_firstPersonControllerObject->m_sprintAccelValue = 0.f;
        }

        if (playerInput->m_overrideTransformForTick || playerInput->m_overrideRotationForTick)
        {
            if (playerInput->m_overrideTransformForTick)
            {
                GetEntity()->GetTransform()->SetWorldTM(playerInput->m_overrideTransform);
                SetOverrideTransformForTick(false);
            }
            else
            {
                GetEntity()->GetTransform()->SetWorldRotationQuaternion(playerInput->m_overrideTransform.GetRotation());
                SetOverrideRotationForTick(false);
            }
            m_firstPersonControllerObject->m_currentHeading = playerInput->m_overrideTransform.GetEulerRadians().GetZ();
            m_firstPersonControllerObject->m_cameraYaw = m_firstPersonControllerObject->m_currentHeading - playerInput->m_yawDelta;
            m_firstPersonControllerObject->m_networkFPCRotationSliceAccumulator = 0.f;
        }

        NetworkFPCControllerNotificationBus::Broadcast(
            &NetworkFPCControllerNotificationBus::Events::OnNetworkTickStart,
            deltaTime,
            m_firstPersonControllerObject->m_isServer,
            GetEntityId());

        if (!m_firstPersonControllerObject->m_isHost)
        {
            const AZ::Quaternion characterRotationQuaternion = AZ::Quaternion::CreateRotationZ(
                m_firstPersonControllerObject->m_currentHeading + playerInput->m_yawDelta + playerInput->m_yawDeltaOvershoot);
            GetEntity()->GetTransform()->SetWorldRotationQuaternion(characterRotationQuaternion);
        }

        const AZ::Vector3 newTranslation = GetNetworkCharacterComponentController()->TryMoveWithVelocity(
            playerInput->m_desiredVelocity, (deltaTime + m_prevDeltaTime) / 2.f);
        m_prevDeltaTime = deltaTime;
        SetCurrentTransform(
            AZ::Transform::CreateFromQuaternionAndTranslation(GetEntity()->GetTransform()->GetWorldRotationQuaternion(), newTranslation));

        NetworkFPCControllerNotificationBus::Broadcast(
            &NetworkFPCControllerNotificationBus::Events::OnNetworkTickFinish,
            deltaTime,
            m_firstPersonControllerObject->m_isServer,
            GetEntityId());
    }

#if AZ_TRAIT_SERVER
    void NetworkFPCController::HandleObtainParentNetEntityId(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZStd::string& strNetEntityId)
    {
        const Multiplayer::NetEntityId providedNetEntityId = static_cast<Multiplayer::NetEntityId>(AZStd::stoull(strNetEntityId));
        const Multiplayer::ConstNetworkEntityHandle providedEntity = Multiplayer::GetNetworkEntityManager()->GetEntity(providedNetEntityId);
        if (providedEntity)
        {
            const Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
            const AZ::EntityId parentId = providedEntity.GetEntity()->GetTransform()->GetParentId();
            const Multiplayer::NetEntityId parentNetEntityId = networkEntityManager->GetNetEntityIdById(parentId);
            const AZStd::string parentStringNetEntityId = AZStd::to_string(parentNetEntityId);
            const AZStd::string childParentStringNetEntityId = strNetEntityId + ',' + parentStringNetEntityId;
            SetChildParentStringNetEntityId(childParentStringNetEntityId);
        }
        else
            SetChildParentStringNetEntityId("");
    }

    // Connect and disconnect events
    void NetworkFPCController::OnConnectionAcquired()
    {
        FirstPersonControllerComponent::GetPlayerStringNetEntityIdsOnServer();
        SetPlayerStringNetEntityIds(FirstPersonControllerComponent::m_playerStringNetEntityIds);
        FirstPersonControllerComponent::m_reacquirePlayerBotStringNetEntityIds = true;
    }
    void NetworkFPCController::OnEndpointDisconnected()
    {
        FirstPersonControllerComponent::GetPlayerStringNetEntityIdsOnServer();
        SetPlayerStringNetEntityIds(FirstPersonControllerComponent::m_playerStringNetEntityIds);
        FirstPersonControllerComponent::m_reacquirePlayerBotStringNetEntityIds = true;
    }
#endif

    // Event Notification methods for use in scripts
    void NetworkFPCController::OnNetworkTickStart(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void NetworkFPCController::OnNetworkTickFinish(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void NetworkFPCController::OnEnableNetworkFPCChanged(const bool& enable)
    {
        m_disabled = !enable;
        m_firstPersonControllerObject->m_networkFPCEnabled = enable;
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = enable;
        if (!m_disabled)
        {
            m_firstPersonControllerObject->IgnoreInputs(true);
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->IgnoreInputs(true);
            AssignConnectInputEvents();
        }
        else
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
            m_firstPersonControllerObject->AssignConnectInputEvents();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->AssignConnectInputEvents();
        }
    }
    void NetworkFPCController::OnPlayerStringNetEntityIdsChanged(const AZStd::vector<AZStd::string>& playerStringNetEntityIds)
    {
        const Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
        m_firstPersonControllerObject->m_playerEntityIds.clear();
        for (AZStd::string playerStringNetEntityId : playerStringNetEntityIds)
        {
            const AZ::Entity* playerEntity =
                networkEntityManager->GetEntity(static_cast<Multiplayer::NetEntityId>(AZStd::stoull(playerStringNetEntityId))).GetEntity();
            if (playerEntity)
                m_firstPersonControllerObject->m_playerEntityIds.push_back(playerEntity->GetId());
        }
    }
    void NetworkFPCController::OnBotStringNetEntityIdsChanged(const AZStd::vector<AZStd::string>& botStringNetEntityIds)
    {
        const Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
        m_firstPersonControllerObject->m_netBotEntityIds.clear();
        for (AZStd::string botStringNetEntityId : botStringNetEntityIds)
        {
            const AZ::Entity* botEntity =
                networkEntityManager->GetEntity(static_cast<Multiplayer::NetEntityId>(AZStd::stoull(botStringNetEntityId))).GetEntity();
            if (botEntity)
                m_firstPersonControllerObject->m_netBotEntityIds.push_back(botEntity->GetId());
        }
    }

    // Request Bus getter and setter methods for use in scripts
    void NetworkFPCController::TryAddVelocityForNetworkTick(const AZ::Vector3& tryVelocity, const float& deltaTime)
    {
        GetNetworkCharacterComponentController()->TryMoveWithVelocity(tryVelocity, deltaTime);
    }
    bool NetworkFPCController::GetAllowAllMovementInputs() const
    {
        return m_allowAllMovementInputs;
    }
    void NetworkFPCController::SetAllowAllMovementInputs(const bool& new_allowAllMovementInputs)
    {
        m_allowAllMovementInputs = new_allowAllMovementInputs;
    }
    bool NetworkFPCController::GetAllowRotationInputs() const
    {
        return m_allowRotationInputs;
    }
    void NetworkFPCController::SetAllowRotationInputs(const bool& new_allowRotationInputs)
    {
        m_allowRotationInputs = new_allowRotationInputs;
    }
    AZ::TimeMs NetworkFPCController::GetHostTimeMs() const
    {
        Multiplayer::INetworkTime* time = Multiplayer::GetNetworkTime();
        return time->GetHostTimeMs();
    }
    bool NetworkFPCController::GetEnabled() const
    {
        return !m_disabled;
    }
    bool NetworkFPCController::GetIsNetEntityRoleAuthority() const
    {
        return IsNetEntityRoleAuthority();
    }
    void NetworkFPCController::SetEnabled(const bool& new_enabled)
    {
        m_disabled = !new_enabled;
        m_firstPersonControllerObject->m_networkFPCEnabled = new_enabled;
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = new_enabled;
        if (!m_disabled)
        {
            m_firstPersonControllerObject->IgnoreInputs(true);
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->IgnoreInputs(true);
            AssignConnectInputEvents();
        }
        else
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
            m_firstPersonControllerObject->AssignConnectInputEvents();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->AssignConnectInputEvents();
        }
    }
} // namespace FirstPersonController
