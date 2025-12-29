/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>
#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Components/CameraBus.h>

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
                ->Field("Jump Delta Angle Speed Factor", &FirstPersonExtrasComponent::m_deltaAngleFactorJump)
                ->Field("Land Delta Angle Speed Factor", &FirstPersonExtrasComponent::m_deltaAngleFactorLand)
                ->Field("Complete Head Angle Land Time", &FirstPersonExtrasComponent::m_completeHeadLandTime)
                ->Attribute(AZ::Edit::Attributes::Suffix, " s")

                // Sprint FoV Group
                ->Field("Sprint FoV", &FirstPersonExtrasComponent::m_sprintFoVEnabled)
                ->Field("FoV Increase When Sprinting", &FirstPersonExtrasComponent::m_sprintFoVDelta)
                ->Attribute(AZ::Edit::Attributes::Suffix, " deg")
                ->Field("Sprint FoV Lerp Time", &FirstPersonExtrasComponent::m_sprintFoVLerpTime)
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
                        "Jump Delta Angle Speed Factor",
                        "Factor that determines how quickly the camera's pitch changes at the start of a jump, "
                        "bigger numbers make it faster.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_deltaAngleFactorLand,
                        "Land Delta Angle Speed Factor",
                        "Factor that determines how quickly the camera's pitch changes when landing, "
                        "bigger numbers make it faster.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_completeHeadLandTime,
                        "Complete Head Angle Land Time",
                        "The time it takes to be in the air for the entire Head Angle Land to apply, "
                        "any time in the air less than this will cause the angle to be proportional to the air time divided by this value.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetJumpHeadTiltEnabled)

                    // Sprint FoV group
                    ->GroupElementToggle("Sprint FoV", &FirstPersonExtrasComponent::m_sprintFoVEnabled)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFoVDelta,
                        "FoV Increase When Sprinting",
                        "The increase in the camera's field of view while sprinting.")
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFoVLerpTime,
                        "Sprint FoV Lerp Time",
                        "The time it takes to get to the maximum field of view when sprinting, and the time it takes to get back.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetSprintFoVEnabled)

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
                ->Event("Get Sprint FoV Enabled", &FirstPersonExtrasComponentRequests::GetSprintFoVEnabled)
                ->Event("Set Sprint FoV Enabled", &FirstPersonExtrasComponentRequests::SetSprintFoVEnabled)
                ->Event("Get Sprint FoV Lerp Time", &FirstPersonExtrasComponentRequests::GetSprintFoVLerpTime)
                ->Event("Get Sprint FoV Lerp Time", &FirstPersonExtrasComponentRequests::SetSprintFoVLerpTime)
                ->Event("Get Sprinting FoV", &FirstPersonExtrasComponentRequests::GetSprintingFoV)
                ->Event("Set Sprinting FoV", &FirstPersonExtrasComponentRequests::SetSprintingFoV)
                ->Event("Get Walking FoV", &FirstPersonExtrasComponentRequests::GetWalkingFoV)
                ->Event("Set Walking FoV", &FirstPersonExtrasComponentRequests::SetWalkingFoV)
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

        // Get access to the FirstPersonControllerComponent and NetwworkFPC objects and their members
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
            if (!m_cameraEntityId.IsValid())
            {
                m_cameraEntityPtr = GetActiveCamera();
                if (m_cameraEntityPtr == nullptr)
                {
                    m_needsHeadbobFallback = true;
                    Camera::CameraNotificationBus::Handler::BusConnect();
                }
            }
            else
            {
                AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
            }

            // Initialize original translation and offsets if pointer is set
            if (m_cameraEntityPtr)
            {
                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
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
        m_cameraEntityPtr = nullptr;
    }

    void FirstPersonExtrasComponent::OnCameraAdded(const AZ::EntityId& cameraId)
    {
        if (!m_cameraEntityId.IsValid())
        {
            m_cameraEntityId = cameraId;
            m_cameraEntityPtr = GetEntityPtr(cameraId);

            if (m_cameraEntityPtr)
            {
                Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);
                // AZ_Printf("First Person Extras Component", "Default camera %s assigned and set as active view.",
                //     m_activeCameraEntity->GetName().empty() ? m_cameraEntityId.ToString().c_str() :
                //     m_activeCameraEntity->GetName().c_str());
            }
            else
            {
                AZ_Warning(
                    "FirstPersonExtrasComponent",
                    false,
                    "Default camera ID %s from CameraNotificationBus is invalid.",
                    cameraId.ToString().c_str());
                m_cameraEntityId = AZ::EntityId();
            }
        }
        // Whenevera a camera is added, use its FoV as the walking FoV value
        Camera::CameraRequestBus::EventResult(m_walkFoV, m_cameraEntityId, &Camera::CameraComponentRequests::GetFovDegrees);
        m_sprintFoV = m_walkFoV + m_sprintFoVDelta;
    }

    void FirstPersonExtrasComponent::OnActiveViewChanged(const AZ::EntityId& activeEntityId)
    {
        if (m_needsHeadbobFallback)
        {
            m_cameraEntityPtr = GetEntityPtr(activeEntityId);
            if (m_cameraEntityPtr != nullptr)
            {
                m_cameraEntityId = activeEntityId;
                Camera::CameraNotificationBus::Handler::BusDisconnect();
                m_needsHeadbobFallback = false;

                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
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

        if (entityId == m_cameraEntityId)
        {
            m_cameraEntityPtr = GetEntityPtr(m_cameraEntityId);
            AZ::EntityBus::Handler::BusDisconnect();
            if (m_cameraEntityPtr)
            {
                m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
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

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), 0);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity)
    {
        if ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
             !m_firstPersonControllerObject->m_isHost) ||
            (entity != GetEntityId()))
        {
            NotAutonomousSoDisconnect();
            return;
        }
        if (!((m_firstPersonControllerObject->m_isHost && server) || (m_firstPersonControllerObject->m_isServer && !server)))
        {
            ProcessInput(((deltaTime + m_prevNetworkFPCDeltaTime) / 2.f), 2);
            m_prevNetworkFPCDeltaTime = deltaTime;
        }
    }

    void FirstPersonExtrasComponent::OnNetworkTickFinish(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entity)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepStart([[maybe_unused]] const float& physicsTimestep)
    {
    }

    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish(const float& physicsTimestep)
    {
        if (m_networkFPCEnabled && !m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
            !m_firstPersonControllerObject->m_isHost)
            return;
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

    void FirstPersonExtrasComponent::PerformSprintFoV(const float& deltaTime)
    {
        if (!m_sprintFoVEnabled)
            return;

        // Obtain whether the character is sprinting and the speed to determine the lerped FoV
        bool sprinting = false;
        FirstPersonControllerComponentRequestBus::EventResult(
            sprinting, GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::GetSprinting);
        const float currentSpeed = m_firstPersonControllerObject->m_applyVelocityXY.GetLength();
        const float sprintScaleForward = m_firstPersonControllerObject->m_sprintScaleForward;
        const float walkSpeed = m_firstPersonControllerObject->m_speed;
        // Scale the FoV based on the current speed, assuming forward is the fastest direction
        if (m_firstPersonControllerObject != nullptr && sprinting &&
            (currentSpeed - walkSpeed) / (sprintScaleForward * walkSpeed - walkSpeed) >= m_sprintFoVTimeAccumulator / m_sprintFoVLerpTime)
        {
            m_sprintFoVTimeAccumulator += deltaTime;
            if (m_sprintFoVTimeAccumulator > m_sprintFoVLerpTime)
                m_sprintFoVTimeAccumulator = m_sprintFoVLerpTime;
        }
        else
        {
            m_sprintFoVTimeAccumulator -= deltaTime;
            if (m_sprintFoVTimeAccumulator < 0.f)
                m_sprintFoVTimeAccumulator = 0.f;
        }
        // Lerp the FoV and apply it
        const float newCameraFoV = AZ::Lerp(m_walkFoV, m_sprintFoV, m_sprintFoVTimeAccumulator / m_sprintFoVLerpTime);
        Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraComponentRequests::SetFovDegrees, newCameraFoV);
    }

    void FirstPersonExtrasComponent::PerformJumpHeadTilt(const float& deltaTime)
    {
        if (!m_jumpHeadTiltEnabled)
            return;

        if (m_tiltJumped)
            m_deltaAngle = deltaTime * m_totalHeadAngle * -m_deltaAngleFactorJump;
        else if (m_tiltLanded)
            m_deltaAngle = deltaTime * m_totalHeadAngle * -m_deltaAngleFactorLand;
        else
            return;

        if (m_moveHeadDown)
        {
            m_currentHeadPitchAngle -= m_deltaAngle;
            m_firstPersonControllerObject->UpdateCameraPitch(-m_deltaAngle, true);
        }
        else
        {
            m_currentHeadPitchAngle += m_deltaAngle;
            m_firstPersonControllerObject->UpdateCameraPitch(m_deltaAngle, true);
        }

        if (m_currentHeadPitchAngle >= 0.f)
        {
            m_moveHeadDown = true;
            m_tiltJumped = false;
            m_tiltLanded = false;
            m_firstPersonControllerObject->UpdateCameraPitch(-m_currentHeadPitchAngle, true);
            m_currentHeadPitchAngle = 0.f;
        }
        else if (m_currentHeadPitchAngle <= m_totalHeadAngle)
        {
            m_moveHeadDown = false;
            m_firstPersonControllerObject->UpdateCameraPitch(m_totalHeadAngle - m_currentHeadPitchAngle, true);
            m_currentHeadPitchAngle = m_totalHeadAngle;
        }
    }

    AZ::Vector3 FirstPersonExtrasComponent::CalculateHeadbobOffset(const float& deltaTime)
    {
        // Walking if FirstPersonController XYs velocity non-zero and grounded
        m_isWalking = !m_firstPersonControllerObject->m_applyVelocityXY.IsZero() && m_firstPersonControllerObject->m_groundClose;

        // Determine states
        const float heading = m_firstPersonControllerObject->m_currentHeading;
        const AZ::Vector2 velocityXY = m_firstPersonControllerObject->m_applyVelocityXY;
        const float localYVelocity = AZ::Quaternion::CreateRotationZ(-heading).TransformVector(AZ::Vector3(velocityXY)).GetY();
        const bool isBackwards =
            m_isWalking && (localYVelocity < -0.5f * m_firstPersonControllerObject->m_speed * m_firstPersonControllerObject->m_backScale);
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

        // When the frequency changes, adjust the input to the sine functions to retain continuity
        if (m_prevEffectiveFrequency != effectiveFrequency)
        {
            m_walkingTime *= m_prevEffectiveFrequency / effectiveFrequency;
            m_prevEffectiveFrequency = effectiveFrequency;
        }

        // Increment m_walkingTime only when walking, reset otherwise
        if (m_isWalking)
            m_walkingTime += deltaTime;
        else
            m_walkingTime = 0.f;

        // Compute offsets using Lemniscate of Gerono (figure-8 pattern for natural sway/bounce).
        const float horizontalOffset = -sinf(m_walkingTime * effectiveFrequency) * effectiveHorizontalAmplitude;
        const float verticalOffset = -sinf(m_walkingTime * effectiveFrequency * 2.f) * effectiveVerticalAmplitude;

        // Combine offets. Horizontal along local X, vertical along local Z.
        return AZ::Vector3(horizontalOffset, 0.f, verticalOffset);
    }

    void FirstPersonExtrasComponent::UpdateHeadbob(const float& deltaTime)
    {
        if (!m_headbobEnabled || !m_cameraEntityPtr)
            return;

        // Compute new headbob offset
        m_headbobOffset = CalculateHeadbobOffset(deltaTime);

        // Bail if no entity
        if (m_cameraEntityPtr == nullptr)
            return;

        auto* headbobEntityTransform = m_cameraEntityPtr->GetTransform();

        // Get the "clean" local translation
        m_cameraTranslationWithoutHeadbob = headbobEntityTransform->GetLocalTM().GetTranslation();
        // Compute the target local translation by adding the new bob offset to the clean position
        const AZ::Vector3 targetLocalTranslation = m_cameraTranslationWithoutHeadbob + m_headbobOffset;
        // Smoothly interpolate from current to target local translation using the provided smoothing factor
        const AZ::Vector3 newLocalTranslation = m_cameraTranslationWithoutHeadbob.Lerp(targetLocalTranslation, m_headbobAttenuation);

        // Set local translation
        headbobEntityTransform->SetLocalTranslation(newLocalTranslation);

        // Update previous offset
        m_previousOffset = newLocalTranslation - m_cameraTranslationWithoutHeadbob;

        // Snap if residual is small
        const AZ::Vector3 lerpResidual = newLocalTranslation - targetLocalTranslation;
        if (lerpResidual.GetLengthSq() <= 1e-6f)
        {
            headbobEntityTransform->SetLocalTranslation(targetLocalTranslation);
            m_previousOffset = m_headbobOffset;
        }
    }

    void FirstPersonExtrasComponent::ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork)
    {
        // Queue up jumps
        QueueJump(deltaTime, tickTimestepNetwork);

        PerformSprintFoV(deltaTime);

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
        if (m_networkFPCEnabled &&
            ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
              !m_firstPersonControllerObject->m_isHost) ||
             !m_firstPersonControllerObject->m_onGroundSoonHit))
            return;
        m_tiltLanded = true;
        m_tiltJumped = false;
        m_moveHeadDown = true;
        float airTime = 0.f;
        FirstPersonControllerComponentRequestBus::EventResult(
            airTime, GetEntityId(), &FirstPersonControllerComponentRequestBus::Events::GetAirTime);
        if (airTime <= m_completeHeadLandTime)
            m_totalHeadAngle = -m_headAngleLand * airTime / m_completeHeadLandTime;
        else
            m_totalHeadAngle = -m_headAngleLand;
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
        if (m_networkFPCEnabled &&
            ((!m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isServer &&
              !m_firstPersonControllerObject->m_isHost) ||
             !m_firstPersonControllerObject->m_onFirstJump))
            return;
        m_tiltJumped = true;
        m_tiltLanded = false;
        m_moveHeadDown = true;
        m_totalHeadAngle = -m_headAngleJump;
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
        return -m_headAngleJump * 360.f / AZ::Constants::TwoPi;
    }
    void FirstPersonExtrasComponent::SetHeadAngleJump(const float& new_headAngleJump)
    {
        m_headAngleJump = new_headAngleJump * AZ::Constants::TwoPi / 360.f;
    }
    float FirstPersonExtrasComponent::GetHeadAngleLand() const
    {
        return -m_headAngleLand * 360.f / AZ::Constants::TwoPi;
    }
    void FirstPersonExtrasComponent::SetHeadAngleLand(const float& new_headAngleLand)
    {
        m_headAngleLand = new_headAngleLand * AZ::Constants::TwoPi / 360.f;
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorJump() const
    {
        return -m_deltaAngleFactorJump;
    }
    void FirstPersonExtrasComponent::SetDeltaAngleFactorJump(const float& new_deltaAngleFactorJump)
    {
        m_deltaAngleFactorJump = new_deltaAngleFactorJump;
    }
    float FirstPersonExtrasComponent::GetDeltaAngleFactorLand() const
    {
        return -m_deltaAngleFactorLand;
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
    bool FirstPersonExtrasComponent::GetSprintFoVEnabled() const
    {
        return m_sprintFoVEnabled;
    }
    void FirstPersonExtrasComponent::SetSprintFoVEnabled(const bool& new_sprintFoVEnabled)
    {
        m_sprintFoVEnabled = new_sprintFoVEnabled;
    }
    float FirstPersonExtrasComponent::GetSprintFoVLerpTime() const
    {
        return m_sprintFoVLerpTime;
    }
    void FirstPersonExtrasComponent::SetSprintFoVLerpTime(const float& new_sprintFoVLerpTime)
    {
        m_sprintFoVLerpTime = new_sprintFoVLerpTime;
    }
    float FirstPersonExtrasComponent::GetSprintingFoV() const
    {
        return m_sprintFoV;
    }
    void FirstPersonExtrasComponent::SetSprintingFoV(const float& new_sprintFoV)
    {
        m_sprintFoV = new_sprintFoV;
        m_sprintFoVDelta = m_sprintFoV - m_walkFoV;
    }
    float FirstPersonExtrasComponent::GetWalkingFoV() const
    {
        return m_walkFoV;
    }
    void FirstPersonExtrasComponent::SetWalkingFoV(const float& new_walkFoV)
    {
        m_walkFoV = new_walkFoV;
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
        return m_cameraEntityId;
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

        m_cameraEntityId = new_headbobEntityId;
        m_cameraEntityPtr = nullptr;

        if (!m_cameraEntityId.IsValid())
        {
            m_cameraEntityPtr = GetEntityPtr(m_cameraEntityId);
            if (m_cameraEntityPtr == nullptr)
            {
                m_needsHeadbobFallback = true;
                Camera::CameraNotificationBus::Handler::BusConnect();
            }
        }
        else
        {
            m_cameraEntityPtr = GetEntityPtr(m_cameraEntityId);
            if (m_cameraEntityPtr == nullptr)
            {
                AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
            }
        }
        // Reset original translation if new entity
        if (m_cameraEntityPtr)
        {
            m_originalCameraTranslation = m_cameraEntityPtr->GetTransform()->GetLocalTranslation();
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
