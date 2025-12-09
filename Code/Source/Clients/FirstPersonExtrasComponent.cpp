/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>
#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonExtrasComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonExtrasComponent, AZ::Component>()
                // Jumping group
                ->Field("Jump Pressed In Air Queue Time Threshold", &FirstPersonExtrasComponent::m_jumpPressedInAirQueueTimeThreshold)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                ->Attribute(AZ::Edit::Attributes::Min, 0.f)

                // Jump Head Tilt group
                ->Field("Jump Head Tilt", &FirstPersonExtrasComponent::m_jumpHeadTiltEnabled)
                ->Field("Head Angle Jump", &FirstPersonExtrasComponent::m_headAngleJump)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Head Angle Land", &FirstPersonExtrasComponent::m_headAngleLand)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Delta Angle Factor Jump", &FirstPersonExtrasComponent::m_deltaAngleFactorJump)
                ->Field("Delta Angle Factor Land", &FirstPersonExtrasComponent::m_deltaAngleFactorLand)
                ->Field("Complete Head Land Time", &FirstPersonExtrasComponent::m_completeHeadLandTime)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")

                // Headbob group
                ->Field("Headbob", &FirstPersonExtrasComponent::m_headbobEnabled)
                ->Field("Frequency", &FirstPersonExtrasComponent::m_headbobFrequency)
                ->Field("Horizontal Amplitude", &FirstPersonExtrasComponent::m_headbobHorizontalAmplitude)
                ->Field("Vertical Amplitude", &FirstPersonExtrasComponent::m_headbobVerticalAmplitude)
                ->Field("Backwards Frequency Scale", &FirstPersonExtrasComponent::m_backwardsFrequencyScale)
                ->Field("Backwards Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_backwardsHorizontalAmplitudeScale)
                ->Field("Backwards Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_backwardsVerticalAmplitudeScale)
                ->Field("Crouch Frequency Scale", &FirstPersonExtrasComponent::m_crouchFrequencyScale)
                ->Field("Crouch Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_crouchHorizontalAmplitudeScale)
                ->Field("Crouch Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_crouchVerticalAmplitudeScale)
                ->Field("Sprint Frequency Scale", &FirstPersonExtrasComponent::m_sprintFrequencyScale)
                ->Field("Sprint Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_sprintHorizontalAmplitudeScale)
                ->Field("Sprint Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_sprintVerticalAmplitudeScale)
                ->Field("Attenuation Factor", &FirstPersonExtrasComponent::m_headbobAttenuation)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonExtrasComponent>(
                      "First Person Extras",
                      "The First Person Extras component provides you with extra features for your first person character controller")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Jumping")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_jumpPressedInAirQueueTimeThreshold,
                        "Jump Pressed In Air Queue Time Threshold",
                        "The duration prior to the character being grounded where pressing and releasing the jump key will be queued up "
                        "for a jump once the character becomes grounded; if the jump key is pressed and released outside of this timing "
                        "window then a jump will not be queued.")
                    ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)

                    // Jump Head Tilt group
                    ->GroupElementToggle("Jump Head Tilt", &FirstPersonExtrasComponent::m_jumpHeadTiltEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headAngleJump,
                        "Head Angle Jump",
                        "The change in the camera's pitch when the character jumps.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headAngleLand,
                        "Head Angle Land",
                        "The change in the camera's pitch when the character lands.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_deltaAngleFactorJump,
                        "Delta Angle Factor Jump",
                        "Factor that determines how quickly the camera's pitch changes at the start of a jump.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_deltaAngleFactorLand,
                        "Delta Angle Factor Land",
                        "Factor that determines how quickly the camera's pitch changes when landing.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_completeHeadLandTime,
                        "Complete Head Land Time",
                        "The time it takes to be in the air for the entire Head Angle Land to apply.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)

                    ->GroupElementToggle("Headbob", &FirstPersonExtrasComponent::m_headbobEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(nullptr, &FirstPersonExtrasComponent::m_headbobFrequency, "Frequency", "Speed of the headbob.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobHorizontalAmplitude,
                        "Horizontal Amplitude",
                        "Left/right headbob distance.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr, &FirstPersonExtrasComponent::m_headbobVerticalAmplitude, "Vertical Amplitude", "Up/down headbob distance.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsFrequencyScale,
                        "Backwards Frequency Scale",
                        "Scale factor for frequency when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsHorizontalAmplitudeScale,
                        "Backwards Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsVerticalAmplitudeScale,
                        "Backwards Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchFrequencyScale,
                        "Crouch Frequency Scale",
                        "Scale factor for frequency when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchHorizontalAmplitudeScale,
                        "Crouch Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchVerticalAmplitudeScale,
                        "Crouch Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFrequencyScale,
                        "Sprint Frequency Scale",
                        "Scale factor for frequency when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintHorizontalAmplitudeScale,
                        "Sprint Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintVerticalAmplitudeScale,
                        "Sprint Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headbobAttenuation,
                        "Attenuation Factor",
                        "Factor to attenuate the magnitude of the oscillation, lower values decrease intensity, a value of 1 does not "
                        "attenuate.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetHeadbobEnabled);
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonExtrasComponentNotificationBus>("FirstPersonExtrasComponentNotificationBus")
                ->Handler<FirstPersonExtrasComponentNotificationHandler>();

            bc->EBus<FirstPersonExtrasComponentRequestBus>("FirstPersonExtrasComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Extras")
                ->Event(
                    "Get Jump Pressed In Air Queue Time Threshold",
                    &FirstPersonExtrasComponentRequests::GetJumpPressedInAirQueueTimeThreshold)
                ->Event(
                    "Set Jump Pressed In Air Queue Time Threshold",
                    &FirstPersonExtrasComponentRequests::SetJumpPressedInAirQueueTimeThreshold)
                ->Event("Get Jump Head Tilt Enabled", &FirstPersonExtrasComponentRequests::GetJumpHeadTiltEnabled)
                ->Event("Set Jump Head Tilt Enabled", &FirstPersonExtrasComponentRequests::SetJumpHeadTiltEnabled)
                ->Event("Get Head Angle Jump", &FirstPersonExtrasComponentRequests::GetHeadAngleJump)
                ->Event("Set Head Angle Jump", &FirstPersonExtrasComponentRequests::SetHeadAngleJump)
                ->Event("Get Head Angle Land", &FirstPersonExtrasComponentRequests::GetHeadAngleLand)
                ->Event("Set Head Angle Land", &FirstPersonExtrasComponentRequests::SetHeadAngleLand)
                ->Event("Set Delta Angle Factor Jump", &FirstPersonExtrasComponentRequests::GetDeltaAngleFactorJump)
                ->Event("Set Delta Angle Factor Jump", &FirstPersonExtrasComponentRequests::SetDeltaAngleFactorJump)
                ->Event("Set Delta Angle Factor Land", &FirstPersonExtrasComponentRequests::GetDeltaAngleFactorLand)
                ->Event("Set Delta Angle Factor Land", &FirstPersonExtrasComponentRequests::SetDeltaAngleFactorLand)
                ->Event("Get Complete Head Land Time", &FirstPersonExtrasComponentRequests::GetCompleteHeadLandTime)
                ->Event("Set Complete Head Land Time", &FirstPersonExtrasComponentRequests::SetCompleteHeadLandTime)
                ->Event("Get Headbob Enabled", &FirstPersonExtrasComponentRequests::GetHeadbobEnabled)
                ->Event("Set Headbob Enabled", &FirstPersonExtrasComponentRequests::SetHeadbobEnabled)
                ->Event("Get Headbob Entity Id", &FirstPersonExtrasComponentRequests::GetHeadbobEntityId)
                ->Event("Set Headbob Entity Id", &FirstPersonExtrasComponentRequests::SetHeadbobEntityId)
                ->Event("Get Camera Translation Without Headbob", &FirstPersonExtrasComponentRequests::GetCameraTranslationWithoutHeadbob)
                ->Event("Get Previous Camera Headbob Offset", &FirstPersonExtrasComponentRequests::GetPreviousOffset);

            bc->Class<FirstPersonExtrasComponent>()->RequestBus("FirstPersonExtrasComponentRequestBus");
        }
    }

    void FirstPersonExtrasComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
        NetworkFPCControllerNotificationBus::Handler::BusConnect(GetEntityId());
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());

        // Get access to the FirstPersonControllerComponent and Netw object and its members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
        m_networkFPCObject = entity->FindComponent<NetworkFPC>();

        // Assign pointer attributes to the associated attributes of the FirstPersonControllerComponent, accessible via friendship
        if (m_firstPersonControllerObject)
        {
            m_jumpValue = &(m_firstPersonControllerObject->m_jumpValue);
            m_scriptJump = &(m_firstPersonControllerObject->m_scriptJump);
            m_grounded = &(m_firstPersonControllerObject->m_grounded);
        }

        // Convert Jump Head Tilt angles to radians
        m_headAngleJump *= AZ::Constants::TwoPi / 360.f;
        m_headAngleLand *= AZ::Constants::TwoPi / 360.f;

        // Assign the FirstPersonExtrasComponent specific inputs
        AssignConnectInputEvents();

        // Headbob activation
        if (m_headbobEnabled)
        {
            // Setup Headbob entity
            if (!m_headbobEntityId.IsValid())
            {
                m_headbobEntityPtr = GetActiveCamera();
                if (m_headbobEntityPtr == nullptr)
                {
                    m_needsHeadbobFallback = true;
                    Camera::CameraNotificationBus::Handler::BusConnect();
                }
            }
            else
            {
                AZ::EntityBus::Handler::BusConnect(m_headbobEntityId);
            }

            // Initialize original translation and offsets if pointer is set
            if (m_headbobEntityPtr)
            {
                m_originalCameraTranslation = m_headbobEntityPtr->GetTransform()->GetLocalTranslation();
                m_previousOffset = AZ::Vector3::CreateZero();
            }
        }
    }

    void FirstPersonExtrasComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        FirstPersonExtrasComponentRequestBus::Handler::BusDisconnect();
        NetworkFPCControllerNotificationBus::Handler::BusDisconnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();

        // Headbob deactivation
        if (m_headbobEnabled)
        {
            if (m_needsHeadbobFallback)
            {
                Camera::CameraNotificationBus::Handler::BusDisconnect();
            }
            AZ::EntityBus::Handler::BusDisconnect();
        }
        m_headbobEntityPtr = nullptr;
    }

    void FirstPersonExtrasComponent::OnActiveViewChanged(const AZ::EntityId& activeEntityId)
    {
        if (m_needsHeadbobFallback)
        {
            m_headbobEntityPtr = GetEntityPtr(activeEntityId);
            if (m_headbobEntityPtr != nullptr)
            {
                m_headbobEntityId = activeEntityId;
                Camera::CameraNotificationBus::Handler::BusDisconnect();
                m_needsHeadbobFallback = false;

                m_originalCameraTranslation = m_headbobEntityPtr->GetTransform()->GetLocalTranslation();
                m_previousOffset = AZ::Vector3::CreateZero();
            }
        }
    }

    void FirstPersonExtrasComponent::OnEntityActivated(const AZ::EntityId& entityId)
    {
        // Get access to the NetworkFPC object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkFPCObject = entity->FindComponent<NetworkFPC>();

        // Determine if the NetworkFPC is enabled
        if (m_networkFPCObject != nullptr)
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
            m_networkFPCEnabled = static_cast<NetworkFPCController*>(m_networkFPCObject->GetController())->GetEnableNetworkFPC();
        }

        if (entityId == m_headbobEntityId)
        {
            m_headbobEntityPtr = GetEntityPtr(m_headbobEntityId);
            AZ::EntityBus::Handler::BusDisconnect();
            if (m_headbobEntityPtr)
            {
                m_originalCameraTranslation = m_headbobEntityPtr->GetTransform()->GetLocalTranslation();
                m_previousOffset = AZ::Vector3::CreateZero();
            }
        }
    }

    AZ::Entity* FirstPersonExtrasComponent::GetActiveCamera() const
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void FirstPersonExtrasComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void FirstPersonExtrasComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
    }

    void FirstPersonExtrasComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void FirstPersonExtrasComponent::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if (m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("First Person Extras Component", false, "Number of input IDs not equal to number of input names!");
        }
        else
        {
            for (auto& it_event : m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
    }

    void FirstPersonExtrasComponent::OnPressed(float value)
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
                // AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonExtrasComponent::OnReleased(float value)
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
                // AZ_Printf("Released", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonExtrasComponent::OnHeld([[maybe_unused]] float value)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepStart([[maybe_unused]] const float& physicsTimestep)
    {
    }

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), 0);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::OnNetworkTickStart(const float& deltaTime, [[maybe_unused]] const bool& server)
    {
        ProcessInput(((deltaTime + m_prevNetworkFPCDeltaTime) / 2.f), 2);
        m_prevNetworkFPCDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::OnNetworkTickFinish([[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish(const float& physicsTimestep)
    {
        ProcessInput(((physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor + m_prevTimestep) / 2.f), 1);
        m_prevTimestep = physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor;
    }

    AZ::Entity* FirstPersonExtrasComponent::GetEntityPtr(AZ::EntityId pointer) const
    {
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(pointer);
    }

    void FirstPersonExtrasComponent::QueueJump(const float& deltaTime, const AZ::u8& tickTimestepNetwork)
    {
        // Bail if the threshold is set to zero
        if (m_jumpPressedInAirQueueTimeThreshold == 0.f)
            return;

        if (tickTimestepNetwork == 0 && !*m_grounded && m_prevJumpValue == 0.f && *m_jumpValue != 0.f)
        {
            // Reset the timer
            m_jumpPressedInAirTimer = 0.f;

            // Queue up the jump when it's pressed while in the air
            m_prevQueueJump = m_queueJump;
            m_queueJump = true;
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_queueJump &&
            !*m_grounded)
        {
            // Increment the timer when the jump is queued
            m_jumpPressedInAirTimer += deltaTime;

            // Check if the timer exceeds the threshold before hitting the ground and if it does then don't perform the jump
            if (m_jumpPressedInAirTimer >= m_jumpPressedInAirQueueTimeThreshold)
            {
                m_jumpPressedInAirTimer = 0.f;
                m_prevQueueJump = false;
                m_queueJump = false;
            }
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_queueJump &&
            *m_grounded)
        {
            // Perform the jump when the ground is hit
            m_prevQueueJump = m_queueJump;
            m_queueJump = false;
            *m_scriptJump = true;

            // Broadcast a notification event that a jump is queued
            FirstPersonExtrasComponentNotificationBus::Broadcast(&FirstPersonExtrasComponentNotificationBus::Events::OnJumpFromQueue);
        }
        else if (
            ((tickTimestepNetwork == 1 && !m_networkFPCEnabled) || (tickTimestepNetwork == 2 && m_networkFPCEnabled)) && m_prevQueueJump &&
            !m_queueJump)
        {
            // Clear the previous queue and script jump variables
            m_prevQueueJump = m_queueJump;
            *m_scriptJump = false;
        }

        // Keep track of the previous jump value
        if (tickTimestepNetwork == 0)
            m_prevJumpValue = *m_jumpValue;
    }

    void FirstPersonExtrasComponent::PerformJumpHeadTilt(const float& deltaTime)
    {
        if (!m_jumpHeadTiltEnabled)
            return;

        if (m_tiltJumped)
        {
            m_deltaAngle = deltaTime * m_totalHeadAngle * m_deltaAngleFactorJump;
        }
        else if (m_tiltLanded)
        {
            m_deltaAngle = deltaTime * m_totalHeadAngle * m_deltaAngleFactorLand;
        }
        else
            return;

        if (m_moveHeadDown)
        {
            m_currentHeadPitchAngle -= m_deltaAngle;
            FirstPersonControllerComponentRequestBus::Event(
                GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::UpdateCameraPitch, -m_deltaAngle, true);
        }
        else
        {
            m_currentHeadPitchAngle += m_deltaAngle;
            FirstPersonControllerComponentRequestBus::Event(
                GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::UpdateCameraPitch, m_deltaAngle, true);
        }

        if (m_currentHeadPitchAngle >= 0.f)
        {
            m_moveHeadDown = true;
            m_tiltJumped = false;
            m_tiltLanded = false;
            FirstPersonControllerComponentRequestBus::Event(
                GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::UpdateCameraPitch, -m_currentHeadPitchAngle, true);
            m_currentHeadPitchAngle = 0.f;
        }
        else if (m_currentHeadPitchAngle <= m_totalHeadAngle)
            m_moveHeadDown = false;
    }

    AZ::Vector3 FirstPersonExtrasComponent::CalculateHeadbobOffset(const float& deltaTime)
    {
        // Walking if FirstPersonController XYs velocity non-zero and grounded
        m_isWalking = !m_firstPersonControllerObject->m_applyVelocityXY.IsZero() && m_firstPersonControllerObject->m_groundClose;

        // Determine states
        const AZ::Vector2 world_local_forward(
            AZ::Quaternion::CreateRotationZ(m_firstPersonControllerObject->m_currentHeading).TransformVector(AZ::Vector3(0.f, 1.f, 0.f)));
        const bool isBackwards = m_isWalking && (m_firstPersonControllerObject->m_applyVelocityXY.Dot(world_local_forward) < 0.0f);
        const bool isCrouching = !m_firstPersonControllerObject->m_standing;
        const bool isSprinting = (!m_firstPersonControllerObject->m_sprintValue == 0.f) &&
            (m_firstPersonControllerObject->m_staminaPercentage > 0.f || !m_firstPersonControllerObject->m_sprintUsesStamina);

        // Compute effective values
        float effectiveFrequency = m_headbobFrequency;
        float effectiveHorizontalAmplitude = m_headbobHorizontalAmplitude;
        float effectiveVerticalAmplitude = m_headbobVerticalAmplitude;

        // Apply backwards scales if moving backwards
        if (isBackwards)
        {
            effectiveFrequency *= m_backwardsFrequencyScale;
            effectiveHorizontalAmplitude *= m_backwardsHorizontalAmplitudeScale;
            effectiveVerticalAmplitude *= m_backwardsVerticalAmplitudeScale;
        }

        // Apply crouch scales if crouching
        if (isCrouching)
        {
            effectiveFrequency *= m_crouchFrequencyScale;
            effectiveHorizontalAmplitude *= m_crouchHorizontalAmplitudeScale;
            effectiveVerticalAmplitude *= m_crouchVerticalAmplitudeScale;
        }

        // Apply sprint scales if sprinting
        if (isSprinting)
        {
            effectiveFrequency *= m_sprintFrequencyScale;
            effectiveHorizontalAmplitude *= m_sprintHorizontalAmplitudeScale;
            effectiveVerticalAmplitude *= m_sprintVerticalAmplitudeScale;
        }

        // Increment m_walkingTime only when walking, reset otherwise.
        if (m_isWalking)
        {
            m_walkingTime += deltaTime;
        }
        else
        {
            m_walkingTime = 0.f;
        }
        // Compute offsets using Lemniscate of Gerono (figure-8 pattern for natural sway/bounce).
        const float horizontalOffset = -sinf(m_walkingTime * effectiveFrequency) * effectiveHorizontalAmplitude;
        const float verticalOffset = -sinf(m_walkingTime * effectiveFrequency * 2.0f) * effectiveVerticalAmplitude;

        // Combine offets. Horizontal along local X, vertical along local Z.
        return AZ::Vector3(horizontalOffset, 0.f, verticalOffset);
    }

    void FirstPersonExtrasComponent::UpdateHeadbob(const float& deltaTime)
    {
        if (!m_headbobEnabled || !m_headbobEntityPtr)
            return;

        // Compute new headbob offset
        m_headbobOffset = CalculateHeadbobOffset(deltaTime);

        // Bail if no entity
        if (m_headbobEntityPtr == nullptr)
            return;

        auto* headbobEntitytransform = m_headbobEntityPtr->GetTransform();

        // Get the headbob entity or camera's current local transform
        const AZ::Transform currentLocalTransform = headbobEntitytransform->GetLocalTM();
        // Calculate the "clean" local translation by removing the previous frame's headbob offset
        m_cameraTranslationWithoutHeadbob = currentLocalTransform.GetTranslation() - m_previousOffset;
        // Compute the target local translation by adding the new bob offset to the clean position
        const AZ::Vector3 targetLocalTranslation = m_cameraTranslationWithoutHeadbob + m_headbobOffset;
        // Smoothly interpolate from current to target local translation using the provided smoothing factor
        const AZ::Vector3 newLocalTranslation = currentLocalTransform.GetTranslation().Lerp(targetLocalTranslation, m_headbobAttenuation);

        // Set local translation
        headbobEntitytransform->SetLocalTranslation(newLocalTranslation);

        // Update previous offset
        if (!m_networkFPCEnabled)
            m_previousOffset = newLocalTranslation - m_cameraTranslationWithoutHeadbob;
        else
            m_previousOffset = newLocalTranslation - currentLocalTransform.GetTranslation();

        // Snap if residual is small
        AZ::Vector3 lerpResidual = newLocalTranslation - targetLocalTranslation;
        if (lerpResidual.GetLengthSq() <= 1e-6f)
        {
            headbobEntitytransform->SetLocalTranslation(targetLocalTranslation);
            m_previousOffset = m_headbobOffset;
        }
    }

    void FirstPersonExtrasComponent::ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork)
    {
        // Queue up jumps
        QueueJump(deltaTime, tickTimestepNetwork);

        if (tickTimestepNetwork == 0)
        {
            if (!m_networkFPCEnabled)
            {
                // Perform Jump Head Tilt
                PerformJumpHeadTilt(deltaTime);
            }
            // Update Headbob
            UpdateHeadbob(deltaTime);
        }
        else if (tickTimestepNetwork == 2)
        {
            // Perform Jump Head Tilt
            PerformJumpHeadTilt(deltaTime);
        }
    }

    // Event Notification methods for use in scripts
    void FirstPersonExtrasComponent::OnJumpFromQueue()
    {
    }

    // Notification Events from the First Person Controller component
    void FirstPersonExtrasComponent::OnNetworkFPCTickStart([[maybe_unused]] const float& deltaTime)
    {
    }
    void FirstPersonExtrasComponent::OnNetworkFPCTickFinish([[maybe_unused]] const float& deltaTime)
    {
    }
    void FirstPersonExtrasComponent::OnGroundHit([[maybe_unused]] const float& fellDistance)
    {
    }
    void FirstPersonExtrasComponent::OnGroundSoonHit([[maybe_unused]] const float& soonFellDistance)
    {
        m_tiltLanded = true;
        m_tiltJumped = false;
        m_moveHeadDown = true;
        float airTime = 0.f;
        FirstPersonControllerComponentRequestBus::EventResult(
            airTime, GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::GetAirTime);
        if (airTime <= m_completeHeadLandTime)
            m_totalHeadAngle = m_headAngleLand * airTime / m_completeHeadLandTime;
        else
            m_totalHeadAngle = m_headAngleLand;
    }
    void FirstPersonExtrasComponent::OnUngrounded()
    {
    }
    void FirstPersonExtrasComponent::OnStartedFalling()
    {
    }
    void FirstPersonExtrasComponent::OnJumpApogeeReached()
    {
    }
    void FirstPersonExtrasComponent::OnStartedMoving()
    {
    }
    void FirstPersonExtrasComponent::OnTargetVelocityReached()
    {
    }
    void FirstPersonExtrasComponent::OnStopped()
    {
    }
    void FirstPersonExtrasComponent::OnTopWalkSpeedReached()
    {
    }
    void FirstPersonExtrasComponent::OnTopSprintSpeedReached()
    {
    }
    void FirstPersonExtrasComponent::OnHeadHit()
    {
    }
    void FirstPersonExtrasComponent::OnCharacterShapecastHitSomething(
        [[maybe_unused]] const AZStd::vector<AzPhysics::SceneQueryHit> characterHits)
    {
    }
    void FirstPersonExtrasComponent::OnVelocityXYObstructed()
    {
    }
    void FirstPersonExtrasComponent::OnCharacterGravityObstructed()
    {
    }
    void FirstPersonExtrasComponent::OnCrouched()
    {
    }
    void FirstPersonExtrasComponent::OnStoodUp()
    {
    }
    void FirstPersonExtrasComponent::OnStoodUpFromJump()
    {
    }
    void FirstPersonExtrasComponent::OnStandPrevented()
    {
    }
    void FirstPersonExtrasComponent::OnStartedCrouching()
    {
    }
    void FirstPersonExtrasComponent::OnStartedStanding()
    {
    }
    void FirstPersonExtrasComponent::OnFirstJump()
    {
        m_tiltJumped = true;
        m_tiltLanded = false;
        m_moveHeadDown = true;
        m_totalHeadAngle = m_headAngleJump;
    }
    void FirstPersonExtrasComponent::OnFinalJump()
    {
    }
    void FirstPersonExtrasComponent::OnStaminaCapped()
    {
    }
    void FirstPersonExtrasComponent::OnStaminaReachedZero()
    {
    }
    void FirstPersonExtrasComponent::OnSprintStarted()
    {
    }
    void FirstPersonExtrasComponent::OnSprintStopped()
    {
    }
    void FirstPersonExtrasComponent::OnCooldownStarted()
    {
    }
    void FirstPersonExtrasComponent::OnCooldownDone()
    {
    }

    // Request Bus getter and setter methods for use in scripts
    float FirstPersonExtrasComponent::GetJumpPressedInAirQueueTimeThreshold() const
    {
        return m_jumpPressedInAirQueueTimeThreshold;
    }
    void FirstPersonExtrasComponent::SetJumpPressedInAirQueueTimeThreshold(const float& new_jumpPressedInAirQueueTimeThreshold)
    {
        if (new_jumpPressedInAirQueueTimeThreshold < 0.f)
            m_jumpPressedInAirQueueTimeThreshold = 0.f;
        else
            m_jumpPressedInAirQueueTimeThreshold = new_jumpPressedInAirQueueTimeThreshold;
    }
    bool FirstPersonExtrasComponent::GetJumpHeadTiltEnabled() const
    {
        return m_jumpHeadTiltEnabled;
    }
    void FirstPersonExtrasComponent::SetJumpHeadTiltEnabled(const bool& new_jumpHeadTiltEnabled)
    {
        m_jumpHeadTiltEnabled = new_jumpHeadTiltEnabled;
    }
    float FirstPersonExtrasComponent::GetHeadAngleJump() const
    {
        return m_headAngleJump * 360.f / AZ::Constants::TwoPi;
    }
    void FirstPersonExtrasComponent::SetHeadAngleJump(const float& new_headAngleJump)
    {
        m_headAngleJump = new_headAngleJump * AZ::Constants::TwoPi / 360.f;
    }
    float FirstPersonExtrasComponent::GetHeadAngleLand() const
    {
        return m_headAngleLand * 360.f / AZ::Constants::TwoPi;
    }
    void FirstPersonExtrasComponent::SetHeadAngleLand(const float& new_headAngleLand)
    {
        m_headAngleLand = new_headAngleLand * AZ::Constants::TwoPi / 360.f;
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorJump() const
    {
        return m_deltaAngleFactorJump;
    }
    void FirstPersonExtrasComponent::SetDeltaAngleFactorJump(const float& new_deltaAngleFactorJump)
    {
        m_deltaAngleFactorJump = new_deltaAngleFactorJump;
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorLand() const
    {
        return m_deltaAngleFactorLand;
    }
    void FirstPersonExtrasComponent::SetDeltaAngleFactorLand(const float& new_deltaAngleFactorLand)
    {
        m_deltaAngleFactorLand = new_deltaAngleFactorLand;
    }
    float FirstPersonExtrasComponent::GetCompleteHeadLandTime() const
    {
        return m_completeHeadLandTime;
    }
    void FirstPersonExtrasComponent::SetCompleteHeadLandTime(const float& new_completeHeadLandTime)
    {
        m_completeHeadLandTime = new_completeHeadLandTime;
    }
    bool FirstPersonExtrasComponent::GetHeadbobEnabled() const
    {
        return m_headbobEnabled;
    }
    void FirstPersonExtrasComponent::SetHeadbobEnabled(const bool& new_headbobEnabled)
    {
        m_headbobEnabled = new_headbobEnabled;
    }
    AZ::EntityId FirstPersonExtrasComponent::GetHeadbobEntityId() const
    {
        return m_headbobEntityId;
    }
    void FirstPersonExtrasComponent::SetHeadbobEntityId(const AZ::EntityId& new_headbobEntityId)
    {
        // Disconnect existing handlers
        if (m_needsHeadbobFallback)
        {
            Camera::CameraNotificationBus::Handler::BusDisconnect();
            m_needsHeadbobFallback = false;
        }
        AZ::EntityBus::Handler::BusDisconnect();

        m_headbobEntityId = new_headbobEntityId;
        m_headbobEntityPtr = nullptr;

        if (!m_headbobEntityId.IsValid())
        {
            m_headbobEntityPtr = GetEntityPtr(m_headbobEntityId);
            if (m_headbobEntityPtr == nullptr)
            {
                m_needsHeadbobFallback = true;
                Camera::CameraNotificationBus::Handler::BusConnect();
            }
        }
        else
        {
            m_headbobEntityPtr = GetEntityPtr(m_headbobEntityId);
            if (m_headbobEntityPtr == nullptr)
            {
                AZ::EntityBus::Handler::BusConnect(m_headbobEntityId);
            }
        }
        // Reset original translation if new entity
        if (m_headbobEntityPtr)
        {
            m_originalCameraTranslation = m_headbobEntityPtr->GetTransform()->GetLocalTranslation();
            m_previousOffset = AZ::Vector3::CreateZero();
        }
    }
    AZ::Vector3 FirstPersonExtrasComponent::GetCameraTranslationWithoutHeadbob() const
    {
        return m_cameraTranslationWithoutHeadbob;
    }
    AZ::Vector3 FirstPersonExtrasComponent::GetPreviousOffset() const
    {
        return m_previousOffset;
    }
    void FirstPersonExtrasComponent::NetworkFPCEnabledIgnoreInputs()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
    }
    void FirstPersonExtrasComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
        Camera::CameraNotificationBus::Handler::BusConnect();
    }
    void FirstPersonExtrasComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
    }
} // namespace FirstPersonController
