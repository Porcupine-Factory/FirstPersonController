/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/FirstPersonExtrasComponent.h>

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

                // HeadBob group
                ->Field("Enable HeadBob", &FirstPersonExtrasComponent::m_enableHeadBob)
                ->Field("HeadBob Entity", &FirstPersonExtrasComponent::m_headBobEntityId)
                ->Field("Frequency", &FirstPersonExtrasComponent::m_headBobFrequency)
                ->Field("Horizontal Amplitude", &FirstPersonExtrasComponent::m_headBobHorizontalAmplitude)
                ->Field("Vertical Amplitude", &FirstPersonExtrasComponent::m_headBobVerticalAmplitude)
                ->Field("Backwards Frequency Scale", &FirstPersonExtrasComponent::m_backwardsFrequencyScale)
                ->Field("Backwards Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_backwardsHorizontalAmplitudeScale)
                ->Field("Backwards Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_backwardsVerticalAmplitudeScale)
                ->Field("Crouch Frequency Scale", &FirstPersonExtrasComponent::m_crouchFrequencyScale)
                ->Field("Crouch Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_crouchHorizontalAmplitudeScale)
                ->Field("Crouch Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_crouchVerticalAmplitudeScale)
                ->Field("Sprint Frequency Scale", &FirstPersonExtrasComponent::m_sprintFrequencyScale)
                ->Field("Sprint Horizontal Amplitude Scale", &FirstPersonExtrasComponent::m_sprintHorizontalAmplitudeScale)
                ->Field("Sprint Vertical Amplitude Scale", &FirstPersonExtrasComponent::m_sprintVerticalAmplitudeScale)
                ->Field("Smoothing", &FirstPersonExtrasComponent::m_headBobSmoothing)
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

                    ->ClassElement(AZ::Edit::ClassElements::Group, "HeadBob")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr, &FirstPersonExtrasComponent::m_enableHeadBob, "Enable HeadBob", "Toggle headbob effect.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headBobEntityId,
                        "Head Bob Entity",
                        "Entity to apply head bob to (e.g., camera). Defaults to active camera if blank.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(nullptr, &FirstPersonExtrasComponent::m_headBobFrequency, "Frequency", "Speed of the headbob.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_headBobHorizontalAmplitude,
                        "Horizontal Amplitude",
                        "Left/right bob distance.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr, &FirstPersonExtrasComponent::m_headBobVerticalAmplitude, "Vertical Amplitude", "Up/down headbob distance.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsFrequencyScale,
                        "Backwards Frequency Scale",
                        "Scale factor for frequency when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsHorizontalAmplitudeScale,
                        "Backwards Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_backwardsVerticalAmplitudeScale,
                        "Backwards Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when moving backwards.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchFrequencyScale,
                        "Crouch Frequency Scale",
                        "Scale factor for frequency when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchHorizontalAmplitudeScale,
                        "Crouch Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_crouchVerticalAmplitudeScale,
                        "Crouch Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when crouching.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintFrequencyScale,
                        "Sprint Frequency Scale",
                        "Scale factor for frequency when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintHorizontalAmplitudeScale,
                        "Sprint Horizontal Amplitude Scale",
                        "Scale factor for horizontal amplitude when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(
                        nullptr,
                        &FirstPersonExtrasComponent::m_sprintVerticalAmplitudeScale,
                        "Sprint Vertical Amplitude Scale",
                        "Scale factor for vertical amplitude when sprinting.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob)
                    ->DataElement(nullptr, &FirstPersonExtrasComponent::m_headBobSmoothing, "Smoothing", "Lower values decrease intensity.")
                    ->Attribute(Visibility, &FirstPersonExtrasComponent::GetEnableHeadBob);
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
                ->Event("Get Enable HeadBob", &FirstPersonExtrasComponentRequests::GetEnableHeadBob)
                ->Event("Get Head Bob Entity Id", &FirstPersonExtrasComponentRequests::GetHeadBobEntityId)
                ->Event("Set Head Bob Entity Id", &FirstPersonExtrasComponentRequests::SetHeadBobEntityId);

            bc->Class<FirstPersonExtrasComponent>()->RequestBus("FirstPersonExtrasComponentRequestBus");
        }
    }

    void FirstPersonExtrasComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusConnect(GetEntityId());
        FirstPersonExtrasComponentRequestBus::Handler::BusConnect(GetEntityId());

        // Get access to the FirstPersonControllerComponent object and its members
        const AZ::Entity* entity = GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();

        // Assign pointer attributes to the associated attributes of the FirstPersonControllerComponent, accessible via friendship
        if (m_firstPersonControllerObject)
        {
            m_jumpValue = &(m_firstPersonControllerObject->m_jumpValue);
            m_scriptJump = &(m_firstPersonControllerObject->m_scriptJump);
            m_grounded = &(m_firstPersonControllerObject->m_grounded);
        }

        // Assign the FirstPersonExtrasComponent specific inputs
        AssignConnectInputEvents();

        // Head Bob activation
        if (m_enableHeadBob)
        {
            // Setup HeadBob entity
            if (!m_headBobEntityId.IsValid())
            {
                m_headBobEntityPtr = GetActiveCamera();
                if (m_headBobEntityPtr == nullptr)
                {
                    m_needsHeadBobFallback = true;
                    Camera::CameraNotificationBus::Handler::BusConnect();
                }
            }
            else
            {
                AZ::EntityBus::Handler::BusConnect(m_headBobEntityId);
            }

            // Initialize original translation and offsets if pointer is set
            if (m_headBobEntityPtr)
            {
                m_originalCameraTranslation = m_headBobEntityPtr->GetTransform()->GetLocalTranslation();
                m_previousOffset = AZ::Vector3::CreateZero();
            }
        }
    }

    void FirstPersonExtrasComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        FirstPersonControllerComponentNotificationBus::Handler::BusDisconnect();
        FirstPersonExtrasComponentRequestBus::Handler::BusDisconnect();

        // Head Bob deactivation
        if (m_enableHeadBob)
        {
            if (m_needsHeadBobFallback)
            {
                Camera::CameraNotificationBus::Handler::BusDisconnect();
            }
            AZ::EntityBus::Handler::BusDisconnect();
        }
        m_headBobEntityPtr = nullptr;
    }

    void FirstPersonExtrasComponent::OnActiveViewChanged(const AZ::EntityId& activeEntityId)
    {
        if (m_needsHeadBobFallback)
        {
            m_headBobEntityPtr = GetEntityPtr(activeEntityId);
            if (m_headBobEntityPtr != nullptr)
            {
                m_headBobEntityId = activeEntityId;
                Camera::CameraNotificationBus::Handler::BusDisconnect();
                m_needsHeadBobFallback = false;

                m_originalCameraTranslation = m_headBobEntityPtr->GetTransform()->GetLocalTranslation();
                m_previousOffset = AZ::Vector3::CreateZero();
            }
        }
    }

    void FirstPersonExtrasComponent::OnEntityActivated(const AZ::EntityId& entityId)
    {
        if (entityId == m_headBobEntityId)
        {
            m_headBobEntityPtr = GetEntityPtr(m_headBobEntityId);
            AZ::EntityBus::Handler::BusDisconnect();
            if (m_headBobEntityPtr)
            {
                m_originalCameraTranslation = m_headBobEntityPtr->GetTransform()->GetLocalTranslation();
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

    void FirstPersonExtrasComponent::OnPhysicsTimestepFinish(const float& physicsTimestep)
    {
        ProcessInput(((physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor + m_prevTimestep) / 2.f), true);
        m_prevTimestep = physicsTimestep * m_firstPersonControllerObject->m_physicsTimestepScaleFactor;
    }

    void FirstPersonExtrasComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), false);
        m_prevDeltaTime = deltaTime;
    }

    void FirstPersonExtrasComponent::QueueJump(const float& deltaTime, const bool& timestepElseTick)
    {
        // Bail if the threshold is set to zero
        if (m_jumpPressedInAirQueueTimeThreshold == 0.f)
            return;

        if (!timestepElseTick && !*m_grounded && m_prevJumpValue == 0.f && *m_jumpValue != 0.f)
        {
            // Reset the timer
            m_jumpPressedInAirTimer = 0.f;

            // Queue up the jump when it's pressed while in the air
            m_prevQueueJump = m_queueJump;
            m_queueJump = true;
        }
        else if (timestepElseTick && m_queueJump && !*m_grounded)
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
        else if (timestepElseTick && m_queueJump && *m_grounded)
        {
            // Perform the jump when the ground is hit
            m_prevQueueJump = m_queueJump;
            m_queueJump = false;
            *m_scriptJump = true;

            // Broadcast a notification event that a jump is queued
            FirstPersonExtrasComponentNotificationBus::Broadcast(&FirstPersonExtrasComponentNotificationBus::Events::OnJumpFromQueue);
        }
        else if (timestepElseTick && m_prevQueueJump && !m_queueJump)
        {
            // Clear the previous queue and script jump variables
            m_prevQueueJump = m_queueJump;
            *m_scriptJump = false;
        }

        // Keep track of the previous jump value
        if (!timestepElseTick)
            m_prevJumpValue = *m_jumpValue;
    }

    void FirstPersonExtrasComponent::SetHeadBobEntity(const AZ::EntityId& id)
    {
        // Disconnect existing handlers
        if (m_needsHeadBobFallback)
        {
            Camera::CameraNotificationBus::Handler::BusDisconnect();
            m_needsHeadBobFallback = false;
        }
        AZ::EntityBus::Handler::BusDisconnect();

        m_headBobEntityId = id;
        m_headBobEntityPtr = nullptr;

        if (!m_headBobEntityId.IsValid())
        {
            m_headBobEntityPtr = GetEntityPtr(m_headBobEntityId);
            if (m_headBobEntityPtr == nullptr)
            {
                m_needsHeadBobFallback = true;
                Camera::CameraNotificationBus::Handler::BusConnect();
            }
        }
        else
        {
            m_headBobEntityPtr = GetEntityPtr(m_headBobEntityId);
            if (m_headBobEntityPtr == nullptr)
            {
                AZ::EntityBus::Handler::BusConnect(m_headBobEntityId);
            }
        }
        // Reset original translation if new entity
        if (m_headBobEntityPtr)
        {
            m_originalCameraTranslation = m_headBobEntityPtr->GetTransform()->GetLocalTranslation();
            m_previousOffset = AZ::Vector3::CreateZero();
        }
    }

    AZ::Vector3 FirstPersonExtrasComponent::CalculateHeadBobOffset(float deltaTime)
    {
        // Walking if FirstPersonController XYs velocity non-zero and grounded
        m_isWalking = !m_firstPersonControllerObject->m_applyVelocityXY.IsZero() && *m_grounded;

        // Determine states
        AZ::Vector2 world_local_forward(
            AZ::Quaternion::CreateRotationZ(m_firstPersonControllerObject->m_currentHeading).TransformVector(AZ::Vector3(0.f, 1.f, 0.f)));
        bool isBackwards = m_isWalking && (m_firstPersonControllerObject->m_applyVelocityXY.Dot(world_local_forward) < 0.0f);
        bool isCrouching = !m_firstPersonControllerObject->m_standing;
        bool isSprinting = (m_firstPersonControllerObject->m_sprintVelocityAdjust > 1.0f) &&
            (m_firstPersonControllerObject->m_standing || m_firstPersonControllerObject->m_sprintWhileCrouched);

        // Compute effective values
        float effectiveFrequency = m_headBobFrequency;
        float effectiveHorizontalAmplitude = m_headBobHorizontalAmplitude;
        float effectiveVerticalAmplitude = m_headBobVerticalAmplitude;

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

        // Get the effective up direction from the FPC
        AZ::Vector3 upDirection = m_firstPersonControllerObject->m_sphereCastsAxisDirectionPose;
        if (upDirection.IsZero())
        {
            upDirection = AZ::Vector3::CreateAxisZ();
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
        float horizontalOffset = -sinf(m_walkingTime * effectiveFrequency) * effectiveHorizontalAmplitude;
        float verticalOffset = -sinf(m_walkingTime * effectiveFrequency * 2.0f) * effectiveVerticalAmplitude;

        // Get camera's local right vector for horizontal alignment.
        m_rightLocalVector =
            AZ::Quaternion(m_headBobEntityPtr->GetTransform()->GetLocalRotationQuaternion()).TransformVector(AZ::Vector3::CreateAxisX());

        // Combine offets. Horizontal along right, vertical along Z.
        return m_rightLocalVector * horizontalOffset + upDirection * verticalOffset;
    }

    void FirstPersonExtrasComponent::UpdateHeadBob(float deltaTime)
    {
        // Compute new headbob offset
        m_offset = CalculateHeadBobOffset(deltaTime);

        // Bail if no entity
        if (m_headBobEntityPtr == nullptr)
            return;

        auto* headBobEntitytransform = m_headBobEntityPtr->GetTransform();

        // Get the headbob entity or camera's current local transform
        AZ::Transform currentLocalTransform = headBobEntitytransform->GetLocalTM();
        // Calculate the "clean" local translation by removing the previous frame's headbob offset
        AZ::Vector3 cleanLocalTranslation = currentLocalTransform.GetTranslation() - m_previousOffset;
        // Compute the target local translation by adding the new bob offset to the clean position
        AZ::Vector3 targetLocalTranslation = cleanLocalTranslation + m_offset;
        // Smoothly interpolate from current to target local translation using the provided smoothing factor
        AZ::Vector3 newLocalTranslation = currentLocalTransform.GetTranslation().Lerp(targetLocalTranslation, m_headBobSmoothing);

        // Set local translation
        headBobEntitytransform->SetLocalTranslation(newLocalTranslation);

        // Update previous offset
        m_previousOffset = newLocalTranslation - cleanLocalTranslation;

        // Snap if residual is small
        AZ::Vector3 lerpResidual = newLocalTranslation - targetLocalTranslation;
        if (lerpResidual.GetLengthSq() <= 1e-6f)
        {
            headBobEntitytransform->SetLocalTranslation(targetLocalTranslation);
            m_previousOffset = m_offset;
        }
    }

    void FirstPersonExtrasComponent::ProcessInput(const float& deltaTime, const bool& timestepElseTick)
    {
        // Queue up jumps
        QueueJump(deltaTime, timestepElseTick);

        // Update headbob only during frame ticks
        if (!timestepElseTick && m_enableHeadBob && m_headBobEntityPtr)
        {
            UpdateHeadBob(deltaTime);
        }
    }

    // Event Notification methods for use in scripts
    void FirstPersonExtrasComponent::OnJumpFromQueue()
    {
    }

    // Notification Events from the First Person Controller component
    void FirstPersonExtrasComponent::OnGroundHit([[maybe_unused]] const float& fellDistance)
    {
    }
    void FirstPersonExtrasComponent::OnGroundSoonHit([[maybe_unused]] const float& soonFellDistance)
    {
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
    bool FirstPersonExtrasComponent::GetEnableHeadBob() const
    {
        return m_enableHeadBob;
    }
    AZ::EntityId FirstPersonExtrasComponent::GetHeadBobEntityId() const
    {
        return m_headBobEntityId;
    }
    void FirstPersonExtrasComponent::SetHeadBobEntityId(const AZ::EntityId& entityId)
    {
        SetHeadBobEntity(entityId);
    }
    AZ::Entity* FirstPersonExtrasComponent::GetEntityPtr(AZ::EntityId pointer) const
    {
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(pointer);
    }
} // namespace FirstPersonController
