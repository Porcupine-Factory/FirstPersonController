#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/InputDeviceId.h>

#include <PhysX/CharacterControllerBus.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonControllerComponent::Reflect(AZ::ReflectContext* rc)
    {
        if(auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonControllerComponent, AZ::Component>()
              // Input Bindings group
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_strYaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_strPitch)
              ->Field("Forward Key", &FirstPersonControllerComponent::m_strForward)
              ->Field("Back Key", &FirstPersonControllerComponent::m_strBack)
              ->Field("Left Key", &FirstPersonControllerComponent::m_strLeft)
              ->Field("Right Key", &FirstPersonControllerComponent::m_strRight)
              ->Field("Sprint Key", &FirstPersonControllerComponent::m_strSprint)
              ->Field("Crouch Key", &FirstPersonControllerComponent::m_strCrouch)
              ->Field("Jump Key", &FirstPersonControllerComponent::m_strJump)

              // Camera Rotation group
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yawSensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitchSensitivity)
              ->Field("Camera Rotation Damp Factor", &FirstPersonControllerComponent::m_rotationDamp)

              // Scale Factors group
              ->Field("Forward Scale", &FirstPersonControllerComponent::m_forwardScale)
              ->Field("Back Scale", &FirstPersonControllerComponent::m_backScale)
              ->Field("Left Scale", &FirstPersonControllerComponent::m_leftScale)
              ->Field("Right Scale", &FirstPersonControllerComponent::m_rightScale)
              ->Field("Sprint Scale", &FirstPersonControllerComponent::m_sprintScale)
              ->Field("Crouch Scale", &FirstPersonControllerComponent::m_crouchScale)

              // X&Y Movement group
              ->Field("Top Walking Speed (m/s)", &FirstPersonControllerComponent::m_speed)
              ->Field("Walking Acceleration (m/s²)", &FirstPersonControllerComponent::m_accel)
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Breaking Factor", &FirstPersonControllerComponent::m_break)

              // Sprint Timing group
              ->Field("Sprint Max Time (sec)", &FirstPersonControllerComponent::m_sprintMaxTime)
              ->Field("Sprint Cooldown (sec)", &FirstPersonControllerComponent::m_sprintCooldownTime)

              // Crouching group
              ->Field("Crouch Distance", &FirstPersonControllerComponent::m_crouchDistance)
              ->Field("Crouch Time", &FirstPersonControllerComponent::m_crouchTime)
              ->Field("Crouch Enable Toggle", &FirstPersonControllerComponent::m_crouchEnableToggle)
              ->Field("Crouch Jump Causes Standing", &FirstPersonControllerComponent::m_crouchJumpCausesStanding)
              ->Field("Crouch Sprint Causes Standing", &FirstPersonControllerComponent::m_crouchSprintCausesStanding)
              ->Field("Crouch Priority When Sprint Pressed", &FirstPersonControllerComponent::m_crouchPriorityWhenSprintPressed)

              // Jumping group
              ->Field("Gravity (m/s²)", &FirstPersonControllerComponent::m_gravity)
              ->Field("Jump Initial Velocity (m/s)", &FirstPersonControllerComponent::m_jumpInitialVelocity)
              ->Field("Jump Held Gravity Factor", &FirstPersonControllerComponent::m_jumpHeldGravityFactor)
              ->Field("Jump Falling Gravity Factor", &FirstPersonControllerComponent::m_jumpFallingGravityFactor)
              ->Field("XY Acceleration Jump Factor (m/s²)", &FirstPersonControllerComponent::m_jumpAccelFactor)
              ->Field("Capsule Grounded Offset (m)", &FirstPersonControllerComponent::m_capsuleOffset)
              ->Field("Capsule Jump Hold Offset (m)", &FirstPersonControllerComponent::m_capsuleJumpHoldOffset)
              ->Field("Enable Double Jump", &FirstPersonControllerComponent::m_doubleJumpEnabled)
              ->Field("Update X&Y Velocity When Ascending", &FirstPersonControllerComponent::m_updateXYAscending)
              ->Field("Update X&Y Velocity When Decending", &FirstPersonControllerComponent::m_updateXYDecending)
              ->Field("Update X&Y Velocity Only When Ground Close", &FirstPersonControllerComponent::m_updateXYOnlyNearGround)

              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonControllerComponent>("First Person Controller",
                    "First person character controller")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Input Bindings")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strYaw,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strPitch,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strForward,
                        "Forward Key", "Key for moving forward")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strBack,
                        "Back Key", "Key for moving back")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strLeft,
                        "Left Key", "Key for moving left")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strRight,
                        "Right Key", "Key for moving right")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strSprint,
                        "Sprint Key", "Key for sprinting")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strCrouch,
                        "Crouch Key", "Key for crouching")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_strJump,
                        "Jump Key", "Key for jumping")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Camera Rotation")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yawSensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitchSensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rotationDamp,
                        "Camera Rotation Damp Factor", "The damp factor applied to the camera rotation")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "X&Y Movement")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_speed,
                        "Top Walking Speed (m/s)", "Speed of the character")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_accel,
                        "Walking Acceleration (m/s²)", "Acceleration")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_decel,
                        "Deceleration Factor", "Deceleration multiplier")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_break,
                        "Breaking Factor", "Breaking multiplier, the final factor is the product of this and the deceleration factor")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Scale Factors")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_forwardScale,
                        "Forward Scale", "Forward movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_backScale,
                        "Back Scale", "Back movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_leftScale,
                        "Left Scale", "Left movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rightScale,
                        "Right Scale", "Right movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintScale,
                        "Sprint Scale", "Sprint scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchScale,
                        "Crouch Scale", "Crouch scale factor")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Sprint Timing")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintMaxTime,
                        "Sprint Max Time (sec)", "The maximum consecutive sprinting time")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprintCooldownTime,
                        "Sprint Cooldown (sec)", "The time required to wait before sprinting again when the maximum consecutive sprint time has been reached")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouching")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchDistance,
                        "Crouch Distance", "Determines the distance the camera will move on the Z axis and the reduction in the PhysX Character Controller's capsule collider height, this number cannot be greater than the capsule's height minus two times its radius")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchTime,
                        "Crouch Time", "Determines the time it takes to complete the crouch")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchEnableToggle,
                        "Crouch Enable Toggle", "Determines whether the crouch key toggles crouching")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchJumpCausesStanding,
                        "Crouch Jump Causes Standing", "Determines whether pressing jump while crouched causes the character to stand up, and then jump once fully standing")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchSprintCausesStanding,
                        "Crouch Sprint Causes Standing", "Determines whether pressing sprint while crouched causes the character to stand up, and then sprint once fully standing")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouchPriorityWhenSprintPressed,
                        "Crouch Priority When Sprint Pressed", "Determines whether pressing crouch while sprint is held causes the character to crouch")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Jumping")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_gravity,
                        "Gravity (m/s²)", "Z Acceleration due to gravity, set this to 0 if you prefer to use the PhysX Character Gameplay component's gravity instead")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpInitialVelocity,
                        "Jump Initial Velocity (m/s)", "Initial jump velocity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpHeldGravityFactor,
                        "Jump Held Gravity Factor", "The factor applied to the character's gravity for the beginning of the jump")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpFallingGravityFactor,
                        "Jump Falling Gravity Factor", "The factor applied to the character's gravity when the Z velocity is negative")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jumpAccelFactor,
                        "XY Acceleration Jump Factor (m/s²)", "X & Y acceleration factor while jumping but still close to the ground")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsuleOffset,
                        "Capsule Grounded Offset (m)", "The capsule's ground detect offset in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsuleJumpHoldOffset,
                        "Capsule Jump Hold Offset (m)", "The capsule's jump hold offset in meters, it is recommended to keep this number less than or equal to the PhysX Character Controller's capsule height")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_doubleJumpEnabled,
                        "Enable Double Jump", "Turn this on if you want to enable double jumping")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYAscending,
                        "Update X&Y Velocity When Ascending", "Determines if the X&Y velocity components will be updated when ascending")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYDecending,
                        "Update X&Y Velocity When Descending", "Determines if the X&Y velocity components will be updated when descending")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_updateXYOnlyNearGround,
                        "Update X&Y Velocity Only When Ground Close", "Determines if the X&Y velocity components will be updated only when the ground close capsule has an intersection, if the ascending and descending options are disabled then this will effectively do nothing");
            }
        }

        if(auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonControllerNotificationBus>("FirstPersonNotificationBus")
                ->Handler<FirstPersonControllerNotificationHandler>();

            bc->EBus<FirstPersonControllerComponentRequestBus>("FirstPersonControllerComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "First Person Controller")
                ->Event("Get Active Camera Id", &FirstPersonControllerComponentRequests::GetActiveCameraId)
                ->Event("Get Grounded", &FirstPersonControllerComponentRequests::GetGrounded)
                ->Event("Set Grounded For Tick", &FirstPersonControllerComponentRequests::SetGroundedForTick)
                ->Event("Get Ground Close", &FirstPersonControllerComponentRequests::GetGroundClose)
                ->Event("Set Ground Close For Tick", &FirstPersonControllerComponentRequests::SetGroundCloseForTick)
                ->Event("Get Air Time", &FirstPersonControllerComponentRequests::GetAirTime)
                ->Event("Get Jump Key Value", &FirstPersonControllerComponentRequests::GetJumpKeyValue)
                ->Event("Get Gravity", &FirstPersonControllerComponentRequests::GetGravity)
                ->Event("Set Gravity", &FirstPersonControllerComponentRequests::SetGravity)
                ->Event("Get Initial Jump Velocity", &FirstPersonControllerComponentRequests::GetInitialJumpVelocity)
                ->Event("Set Initial Jump Velocity", &FirstPersonControllerComponentRequests::SetInitialJumpVelocity)
                ->Event("Get Double Jump", &FirstPersonControllerComponentRequests::GetDoubleJump)
                ->Event("Set Double Jump", &FirstPersonControllerComponentRequests::SetDoubleJump)
                ->Event("Get Capsule Offset", &FirstPersonControllerComponentRequests::GetCapsuleOffset)
                ->Event("Set Capsule Offset", &FirstPersonControllerComponentRequests::SetCapsuleOffset)
                ->Event("Get Capsule Jump Hold Offset", &FirstPersonControllerComponentRequests::GetCapsuleJumpHoldOffset)
                ->Event("Set Capsule Jump Hold Offset", &FirstPersonControllerComponentRequests::SetCapsuleJumpHoldOffset)
                ->Event("Get Max Grounded Angle Degrees", &FirstPersonControllerComponentRequests::GetMaxGroundedAngleDegrees)
                ->Event("Set Max Grounded Angle Degrees", &FirstPersonControllerComponentRequests::SetMaxGroundedAngleDegrees)
                ->Event("Get Top Walk Speed", &FirstPersonControllerComponentRequests::GetTopWalkSpeed)
                ->Event("Set Top Walk Speed", &FirstPersonControllerComponentRequests::SetTopWalkSpeed)
                ->Event("Get Walk Acceleration", &FirstPersonControllerComponentRequests::GetWalkAcceleration)
                ->Event("Set Walk Acceleration", &FirstPersonControllerComponentRequests::SetWalkAcceleration)
                ->Event("Get Walk Deceleration", &FirstPersonControllerComponentRequests::GetWalkDeceleration)
                ->Event("Set Walk Deceleration", &FirstPersonControllerComponentRequests::SetWalkDeceleration)
                ->Event("Get Walk Break", &FirstPersonControllerComponentRequests::GetWalkBreak)
                ->Event("Set Walk Break", &FirstPersonControllerComponentRequests::SetWalkBreak)
                ->Event("Get Sprint Scale", &FirstPersonControllerComponentRequests::GetSprintScale)
                ->Event("Set Sprint Scale", &FirstPersonControllerComponentRequests::SetSprintScale)
                ->Event("Get Crouch Scale", &FirstPersonControllerComponentRequests::GetCrouchScale)
                ->Event("Set Crouch Scale", &FirstPersonControllerComponentRequests::SetCrouchScale)
                ->Event("Get Sprint Held Time", &FirstPersonControllerComponentRequests::GetSprintHeldTime)
                ->Event("Set Sprint Held Time", &FirstPersonControllerComponentRequests::SetSprintHeldTime)
                ->Event("Get Sprint Cooldown", &FirstPersonControllerComponentRequests::GetSprintCooldown)
                ->Event("Set Sprint Cooldown", &FirstPersonControllerComponentRequests::SetSprintCooldown)
                ->Event("Get Sprint Pause Time", &FirstPersonControllerComponentRequests::GetSprintPauseTime)
                ->Event("Set Sprint Pause Time", &FirstPersonControllerComponentRequests::SetSprintPauseTime)
                ->Event("Get Crouching", &FirstPersonControllerComponentRequests::GetCrouching)
                ->Event("Set Crouching", &FirstPersonControllerComponentRequests::SetCrouching)
                ->Event("Get Crouch Distance", &FirstPersonControllerComponentRequests::GetCrouchDistance)
                ->Event("Set Crouch Distance", &FirstPersonControllerComponentRequests::SetCrouchDistance)
                ->Event("Get Crouch Time", &FirstPersonControllerComponentRequests::GetCrouchTime)
                ->Event("Set Crouch Time", &FirstPersonControllerComponentRequests::SetCrouchTime)
                ->Event("Get Crouch Enable Toggle", &FirstPersonControllerComponentRequests::GetCrouchEnableToggle)
                ->Event("Set Crouch Enable Toggle", &FirstPersonControllerComponentRequests::SetCrouchEnableToggle)
                ->Event("Get Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchJumpCausesStanding)
                ->Event("Set Crouch Jump Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchJumpCausesStanding)
                ->Event("Get Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::GetCrouchSprintCausesStanding)
                ->Event("Set Crouch Sprint Causes Standing", &FirstPersonControllerComponentRequests::SetCrouchSprintCausesStanding)
                ->Event("Get Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::GetCrouchPriorityWhenSprintPressed)
                ->Event("Set Crouch Priority When Sprint Pressed", &FirstPersonControllerComponentRequests::SetCrouchPriorityWhenSprintPressed)
                ->Event("Get Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::GetCameraPitchSensitivity)
                ->Event("Set Camera Pitch Sensitivity", &FirstPersonControllerComponentRequests::SetCameraPitchSensitivity)
                ->Event("Get Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::GetCameraYawSensitivity)
                ->Event("Set Camera Yaw Sensitivity", &FirstPersonControllerComponentRequests::SetCameraYawSensitivity)
                ->Event("Get Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::GetCameraRotationDampFactor)
                ->Event("Set Camera Rotation Damp Factor", &FirstPersonControllerComponentRequests::SetCameraRotationDampFactor)
                ->Event("Update Camera Pitch", &FirstPersonControllerComponentRequests::UpdateCameraPitch)
                ->Event("Update Camera Yaw", &FirstPersonControllerComponentRequests::UpdateCameraYaw)
                ->Event("Get Character Heading", &FirstPersonControllerComponentRequests::GetHeading);

            bc->Class<FirstPersonControllerComponent>()->RequestBus("FirstPersonControllerComponentRequestBus");
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
        UpdateJumpTime();

        if(m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Printf("FirstPersonControllerComponent",
                      "ERROR: Number of input IDs not equal to number of input names!");
        }
        else
        {
            for(auto& it_event : m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
        AZ::TickBus::Handler::BusConnect();

        InputChannelEventListener::Connect();
        // Attempting to allow all possible input events through without filtering anything out
        // This may not be necessary
        AZStd::shared_ptr<AzFramework::InputChannelEventFilterInclusionList> filter;
        AzFramework::InputChannelEventListener::SetFilter(filter);

        FirstPersonControllerComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void FirstPersonControllerComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        InputChannelEventListener::Disconnect();
        FirstPersonControllerComponentRequestBus::Handler::BusDisconnect();
    }

    void FirstPersonControllerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    void FirstPersonControllerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void FirstPersonControllerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void FirstPersonControllerComponent::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        if(*inputId == m_SprintEventId)
        {
            m_sprintValue = m_sprintPressedValue = value * m_sprintScale;
        }

        for(auto& it_event : m_controlMap)
        {
            if(*inputId == *(it_event.first) && !(*(it_event.first) == m_SprintEventId))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                //AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        for(auto& it_event : m_controlMap)
        {
            if(*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                //AZ_Printf("Released", it_event.first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
        {
            return;
        }

        if(*inputId == m_RotateYawEventId)
        {
            m_yawValue = value;
        }
        else if(*inputId == m_RotatePitchEventId)
        {
            m_pitchValue = value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if(*inputId == m_SprintEventId)
        {
            m_sprintValue = m_sprintPressedValue = value * m_sprintScale;
        }
    }

    bool FirstPersonControllerComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
    {
        const AzFramework::InputDeviceId& deviceId = inputChannel.GetInputDevice().GetInputDeviceId();

        // TODO: Implement gamepad support
        //AZ_Printf("", "OnInputChannelEventFiltered");
        if(AzFramework::InputDeviceGamepad::IsGamepadDevice(deviceId))
            OnGamepadEvent(inputChannel);

        return false;
    }

    void FirstPersonControllerComponent::OnGamepadEvent(const AzFramework::InputChannel& inputChannel)
    {
        // TODO: Implement gamepad support
        const AzFramework::InputChannelId& channelId = inputChannel.GetInputChannelId();

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LR)
        {
            m_rightValue = inputChannel.GetValue();
            m_leftValue = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LL)
        {
            m_rightValue = 0.f;
            m_leftValue = -1.f*inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LU)
        {
            m_forwardValue = inputChannel.GetValue();
            m_backValue = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LD)
        {
            m_forwardValue = 0.f;
            m_backValue = -1.f*inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RY)
        {
            m_cameraRotationAngles[0] = inputChannel.GetValue() * m_pitchSensitivity;
            m_rotatingPitchViaScriptGamepad = true;
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RX)
        {
            m_cameraRotationAngles[2] = -1.f*inputChannel.GetValue() * m_yawSensitivity;
            m_rotatingYawViaScriptGamepad = true;
        }
    }

    void FirstPersonControllerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime);
    }

    AZ::Entity* FirstPersonControllerComponent::GetActiveCamera() const
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId,
            &Camera::CameraSystemRequestBus::Events::GetActiveCamera);

        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void FirstPersonControllerComponent::SlerpRotation(const float& deltaTime)
    {
        // Multiply by -1 since moving the mouse up produces a negative value from the input bus
        if(!m_rotatingPitchViaScriptGamepad)
            m_cameraRotationAngles[0] = -1.f * m_pitchValue * m_pitchSensitivity;
        else
            m_rotatingPitchViaScriptGamepad = false;

        // Multiply by -1 since moving the mouse to the right produces a positive value
        // but a positive rotation about Z is counterclockwise
        if(!m_rotatingYawViaScriptGamepad)
            m_cameraRotationAngles[2] = -1.f * m_yawValue * m_yawSensitivity;
        else
            m_rotatingYawViaScriptGamepad = false;

        const AZ::Quaternion targetLookRotationDelta = AZ::Quaternion::CreateFromEulerAnglesRadians(
            AZ::Vector3::CreateFromFloat3(m_cameraRotationAngles));

        if(m_rotationDamp*deltaTime <= 1.f)
            m_newLookRotationDelta = m_newLookRotationDelta.Slerp(targetLookRotationDelta, m_rotationDamp*deltaTime);
        else
            m_newLookRotationDelta = targetLookRotationDelta;
    }

    void FirstPersonControllerComponent::UpdateRotation(const float& deltaTime)
    {
        AZ::TransformInterface* t = GetEntity()->GetTransform();

        SlerpRotation(deltaTime);
        const AZ::Vector3 newLookRotationDelta = m_newLookRotationDelta.GetEulerRadians();

        t->RotateAroundLocalZ(newLookRotationDelta.GetZ());

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        float currentPitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(currentPitch) <= HalfPi ||
           currentPitch >= HalfPi && newLookRotationDelta.GetX() < 0.f ||
           currentPitch <= -HalfPi && newLookRotationDelta.GetX() > 0.f)
        {
            t->RotateAroundLocalX(newLookRotationDelta.GetX());
            currentPitch = t->GetLocalRotation().GetX();
        }
        if(abs(currentPitch) > HalfPi)
        {
            if(currentPitch > HalfPi)
                t->RotateAroundLocalX(HalfPi - currentPitch);
            else
                t->RotateAroundLocalX(-HalfPi - currentPitch);
        }

        m_currentHeading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();
    }

    AZ::Vector3 FirstPersonControllerComponent::LerpVelocity(const AZ::Vector3& targetVelocity, const float& deltaTime)
    {
        float totalLerpTime = m_lastAppliedVelocity.GetDistance(targetVelocity)/m_accel;

        // Apply the sprint factor to the acceleration (dt) based on the sprint having been (recently) pressed
        const float lastLerpTime = m_lerpTime;

        float lerpDeltaTime = m_sprintIncrementTime > 0.f ? deltaTime * (1.f + (m_sprintPressedValue-1.f) * m_sprintAccelAdjust) : deltaTime;
        lerpDeltaTime *= m_grounded ? 1.f : m_jumpAccelFactor;

        m_lerpTime += lerpDeltaTime;

        if(m_lerpTime >= totalLerpTime)
            m_lerpTime = totalLerpTime;

        // Lerp the velocity from the last applied velocity to the target velocity
        AZ::Vector3 newVelocity = m_lastAppliedVelocity.Lerp(targetVelocity, m_lerpTime / totalLerpTime);

        // Decelerate at a different rate than the acceleration
        if(newVelocity.GetLength() < m_applyVelocity.GetLength())
        {
            // Get the current velocity vector with respect to the world coordinates
            const AZ::Vector3 applyVelocityWorld = AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(m_applyVelocity);

            float decelerationFactor = m_decel;

            // Compare the direction of the current velocity vector against the desired direction
            // and if it's greater than 90 degrees then decelerate even more
            if(targetVelocity.GetLength() != 0.f && abs(applyVelocityWorld.Angle(targetVelocity)) > AZ::Constants::HalfPi)
                decelerationFactor = m_break;

            // Use the deceleration factor to get the lerp time closer to the total lerp time at a faster rate
            m_lerpTime = lastLerpTime + lerpDeltaTime * decelerationFactor;

            if(m_lerpTime >= totalLerpTime)
                m_lerpTime = totalLerpTime;

            newVelocity = m_lastAppliedVelocity.Lerp(targetVelocity, m_lerpTime / totalLerpTime);
        }

        return newVelocity;
    }

    void FirstPersonControllerComponent::SprintManager(const AZ::Vector3& targetVelocity, const float& deltaTime)
    {
        // Cause the character to stand if trying to sprint while crouched and the setting is enabled
        if(m_crouchSprintCausesStanding && m_sprintValue != 1.f && m_crouching)
            m_crouching = false;

        // The sprint value should never be 0 and it shouldn't be applied if you're trying to moving backwards
        if(m_sprintValue == 0.f
           || !m_standing
           || !m_grounded
           || (!m_applyVelocity.GetY() && !m_applyVelocity.GetX())
           || (m_sprintValue != 1.f
               && ((!m_forwardValue && !m_leftValue && !m_rightValue) ||
                   (!m_forwardValue && -m_leftValue == m_rightValue) ||
                   (targetVelocity.GetY() < 0.f)) ))
            m_sprintValue = 1.f;

        // Set the sprint value to 1 and reset the counter if there is no movement
        if(!m_applyVelocity.GetY() && !m_applyVelocity.GetX())
        {
            m_sprintValue = 1.f;
            m_sprintIncrementTime = 0.f;
        }

        const float totalSprintTime = ((m_sprintValue-1.f)*m_speed)/m_accel;

        // If the sprint key is pressed then increment the sprint counter
        if(m_sprintValue != 1.f && m_sprintHeldDuration < m_sprintMaxTime && m_sprintCooldown == 0.f)
        {
            // Sprint adjustment factor based on the angle of the target velocity
            // with respect to their frame of reference
            m_sprintVelocityAdjust = 1.f - targetVelocity.Angle(AZ::Vector3::CreateAxisY())/(AZ::Constants::HalfPi);
            m_sprintAccelAdjust = m_sprintVelocityAdjust;

            m_sprintIncrementTime += deltaTime;
            m_sprintHeldDuration += deltaTime * m_sprintVelocityAdjust;

            m_sprintDecrementing = false;

            if(m_sprintIncrementTime > totalSprintTime)
                m_sprintIncrementTime = totalSprintTime;
        }
        // Otherwise if the sprint key isn't pressed then decrement the sprint counter
        else if(m_sprintValue == 1.f || m_sprintHeldDuration >= m_sprintMaxTime || m_sprintCooldown != 0.f)
        {
            // Set the sprint velocity adjust to 0
            m_sprintVelocityAdjust = 0.f;

            m_sprintIncrementTime -= deltaTime;
            if(m_sprintIncrementTime < 0.f)
                m_sprintIncrementTime = 0.f;

            // When the sprint held duration exceeds the maximum sprint time then initiate the cooldown period
            if(m_sprintHeldDuration >= m_sprintMaxTime && m_sprintCooldown == 0.f)
            {
                m_sprintHeldDuration = 0.f;
                m_sprintCooldown = m_sprintCooldownTime;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSprintCooldown);
            }
            else if(m_sprintCooldown != 0.f)
            {
                m_sprintCooldown -= deltaTime;
                if(m_sprintCooldown < 0.f)
                    m_sprintCooldown = 0.f;
            }
            // If the sprint cooldown time is longer than the maximum sprint duration
            // then apply a pause based on the difference between the two, times the ratio of
            // how long sprint was held divided by the maximum sprint duration prior to
            // decrementing the sprint held duration
            else if(m_sprintCooldownTime > m_sprintMaxTime)
            {
                m_sprintDecrementPause -= deltaTime;

                if(m_sprintHeldDuration > 0.f && !m_sprintDecrementing)
                {
                    m_sprintDecrementPause = (m_sprintCooldownTime - m_sprintMaxTime)
                                                *(m_sprintHeldDuration/m_sprintMaxTime);
                    // m_sprintPrevDecrementPause is not used here, but setting it for potential future use
                    m_sprintPrevDecrementPause = m_sprintDecrementPause;
                    m_sprintDecrementing = true;
                }

                if(m_sprintDecrementPause <= 0.f)
                {
                    m_sprintHeldDuration -= deltaTime;
                    m_sprintDecrementPause = 0.f;
                    if(m_sprintHeldDuration <= 0.f)
                    {
                        m_sprintHeldDuration = 0.f;
                        m_sprintDecrementing = false;
                    }
                }
            }
            else if(m_sprintCooldownTime <= m_sprintMaxTime)
            {
                m_sprintDecrementPause -= deltaTime;
                if(m_sprintHeldDuration > 0.f && !m_sprintDecrementing)
                {
                    // Making the m_sprintDecrementPause a factor of 0.1 here is somewhat arbitrary,
                    // this can be set to any other desired number if you have
                    // m_sprintCooldownTime <= m_sprintMaxTime.
                    // The decrement time here is also set based on the cooldown time and the ratio of the
                    // held duration divided by the maximum consecutive sprint time so that the pause is longer
                    // if you recently sprinted for a while.
                    m_sprintDecrementPause = 0.1f * m_sprintCooldownTime * m_sprintHeldDuration / m_sprintMaxTime;
                    m_sprintPrevDecrementPause = m_sprintDecrementPause;
                    m_sprintDecrementing = true;
                }

                if(m_sprintDecrementPause <= 0.f)
                {
                    // Decrement the held duration by a factor of the ratio of the max sprint time plus the
                    // previous decrement pause time, divided by the cooldown time
                    // so there is not an incentive to elapse it.
                    // This makes it so that the held duration decrements and gets back to 0 at the same rate
                    // as if you were to just allow it to elapse.
                    m_sprintHeldDuration -= deltaTime * ((m_sprintMaxTime+m_sprintPrevDecrementPause)/m_sprintCooldownTime);
                    m_sprintDecrementPause = 0.f;
                    if(m_sprintHeldDuration <= 0.f)
                    {
                        m_sprintHeldDuration = 0.f;
                        m_sprintDecrementing = false;
                    }
                }
            }
        }
    }

    void FirstPersonControllerComponent::CrouchManager(const float& deltaTime)
    {
        AZ::TransformInterface* cameraTransform = m_activeCameraEntity->GetTransform();

        if(m_crouchEnableToggle && m_crouchPrevValue == 0.f && m_crouchValue == 1.f)
        {
            m_crouching = !m_crouching;
        }
        else if(!m_crouchEnableToggle)
        {
            if(m_crouchValue != 0.f)
                m_crouching = true;
            else
                m_crouching = false;
        }

        // If the crouch key takes priority when the sprint key is held and we're attempting to crouch
        // while the sprint key is being pressed then stop the sprinting and continue crouching
        if(m_crouchPriorityWhenSprintPressed
                && m_sprintValue != 1.f
                && m_crouching
                && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
            m_sprintValue = 1.f;
        // Otherwise if the crouch key does not take priority when the sprint key is held,
        // and we are attempting to crouch while the sprint key is held, then do not crouch
        else if(!m_crouchPriorityWhenSprintPressed
            && m_sprintValue != 1.f
            && m_crouching
            && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
           m_crouching = false;

        //AZ_Printf("", "m_crouching = %s", m_crouching ? "true" : "false");

        // Crouch down
        if(m_crouching && m_cameraLocalZTravelDistance > -1.f * m_crouchDistance)
        {
            if(m_standing)
                m_standing = false;

            float cameraTravelDelta = -1.f * m_crouchDistance * deltaTime / m_crouchTime;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            if(m_cameraLocalZTravelDistance <= -1.f * m_crouchDistance)
            {
                cameraTravelDelta += abs(m_cameraLocalZTravelDistance) - m_crouchDistance;
                m_cameraLocalZTravelDistance = -1.f * m_crouchDistance;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnCrouched);
            }

            // Adjust the height of the collider capsule based on the crouching height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Subtract the distance to get down to the crouching height
            m_capsuleHeight += cameraTravelDelta;
            //AZ_Printf("", "Crouching capsule height = %.10f", m_capsuleHeight);

            // Recalculate the ground detect shapecast capsule's offset
            m_capsuleOffsetTranslation = m_capsuleHeight/2.f - m_capsuleOffset;
            m_capsuleJumpHoldOffsetTranslation = m_capsuleHeight/2.f - m_capsuleJumpHoldOffset;

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleHeight);

            cameraTransform->SetLocalZ(cameraTransform->GetLocalZ() + cameraTravelDelta);
        }
        // Stand up
        else if(!m_crouching && m_cameraLocalZTravelDistance != 0.f)
        {
            // Create a shapecast capsule that will be used to detect whether there is an obstruction
            // above the players head, and prevent them from fully standing up if there is
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            // Rotate the pose by 90 degrees on the Y axis since by default the capsule's height
            // is oriented along the X axis when we want it oriented along the Z axis
            AZ::Transform capsuleIntersectionPose = AZ::Transform::CreateRotationY(AZ::Constants::HalfPi);
            // Move the capsule to the location of the character and apply the Z offset

            capsuleIntersectionPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleHeight/2.f + m_capsuleOffset));

            AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsuleRadius,
                m_capsuleHeight,
                capsuleIntersectionPose,
                AZ::Vector3(0.f, 0.f, 1.f),
                0.f,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                AzPhysics::CollisionGroup::All,
                nullptr);

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

            // Disregard intersections with the character's collider and its child entities,
            auto selfChildEntityCheck = [this](AzPhysics::SceneQueryHit& hit)
                {
                    if(hit.m_entityId == GetEntityId())
                        return true;

                    // Obtain the child IDs if we don't already have them
                    if(!m_obtainedChildIds)
                    {
                        AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                        m_obtainedChildIds = true;
                    }

                    for(AZ::EntityId id: m_children)
                    {
                        if(hit.m_entityId == id)
                            return true;
                    }

                    return false;
                };

            AZStd::erase_if(hits.m_hits, selfChildEntityCheck);

            // Bail if something is detected above the player
            if(hits)
                return;

            float cameraTravelDelta = m_crouchDistance * deltaTime / m_crouchTime;
            m_cameraLocalZTravelDistance += cameraTravelDelta;

            if(m_cameraLocalZTravelDistance >= 0.f)
            {
                cameraTravelDelta -= m_cameraLocalZTravelDistance;
                m_cameraLocalZTravelDistance = 0.f;
                m_standing = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnStoodUp);
            }

            // Adjust the height of the collider capsule based on the standing height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Add the distance to get back to the standing height
            m_capsuleHeight += cameraTravelDelta;
            //AZ_Printf("", "Standing capsule height = %.10f", m_capsuleHeight);

            // Recalculate the ground detect shapecast capsule's offset
            m_capsuleOffsetTranslation = m_capsuleHeight/2.f - m_capsuleOffset;
            m_capsuleJumpHoldOffsetTranslation = m_capsuleHeight/2.f - m_capsuleJumpHoldOffset;

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsuleHeight);

            cameraTransform->SetLocalZ(cameraTransform->GetLocalZ() + cameraTravelDelta);
        }

        m_crouchPrevValue = m_crouchValue;
    }

    void FirstPersonControllerComponent::UpdateVelocityXY(const float& deltaTime)
    {
        float forwardBack = m_forwardValue * m_forwardScale + m_backValue * m_backScale;
        float leftRight = m_leftValue * m_leftScale + m_rightValue * m_rightScale;

        // Remove the scale factor since it's going to be applied after the normalization
        if(forwardBack >= 0.f)
            forwardBack /= m_forwardScale;
        else
            forwardBack /= m_backScale;

        if(leftRight >= 0.f)
            leftRight /= m_rightScale;
        else
            leftRight /= m_leftScale;

        AZ::Vector3 targetVelocity = AZ::Vector3(leftRight, forwardBack, 0.f);

        // Normalize the vector if its magnitude is greater than 1 and then scale it
        if((forwardBack || leftRight) && sqrt(forwardBack*forwardBack + leftRight*leftRight) > 1.f)
            targetVelocity.Normalize();

        if(targetVelocity.GetY() >= 0.f)
            targetVelocity.SetY(targetVelocity.GetY() * m_forwardScale);
        else
            targetVelocity.SetY(targetVelocity.GetY() * m_backScale);

        if(targetVelocity.GetX() >= 0.f)
            targetVelocity.SetX(targetVelocity.GetX() * m_rightScale);
        else
            targetVelocity.SetX(targetVelocity.GetX() * m_leftScale);

        // Call the sprint manager
        SprintManager(targetVelocity, deltaTime);

        // Apply the speed and sprint factor
        if(m_standing)
            targetVelocity *= m_speed * (1.f + (m_sprintValue-1.f) * m_sprintVelocityAdjust);
        // Don't apply the sprint factor when crouching
        else
            targetVelocity *= m_speed * m_crouchScale;

        // Obtain the last applied velocity if the target velocity changed
        if(m_prevTargetVelocity != targetVelocity)
        {
            // Set the previous target velocity to the new one
            m_prevTargetVelocity = targetVelocity;

            // Store the last applied velocity to be used for the lerping
            m_lastAppliedVelocity = AZ::Quaternion::CreateRotationZ(-m_currentHeading).TransformVector(m_applyVelocity);

            // Reset the lerp time since the target velocity changed
            m_lerpTime = 0.f;
        }

        // Rotate the target velocity vector so that it can be compared against the applied velocity
        const AZ::Vector3 targetVelocityWorld = AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(targetVelocity);

        // Lerp to the velocity if we're not already there
        if(m_applyVelocity != targetVelocityWorld)
            m_applyVelocity = AZ::Quaternion::CreateRotationZ(m_currentHeading).TransformVector(LerpVelocity(targetVelocity, deltaTime));

        // Debug print statements to observe the velocity, acceleration, and position
        //AZ_Printf("", "m_currentHeading = %.10f", m_currentHeading);
        //AZ_Printf("", "atan(m_applyVelocity.GetY()/m_applyVelocity.GetX()) = %.10f", atan(m_applyVelocity.GetY()/m_applyVelocity.GetX()));
        //AZ_Printf("", "m_applyVelocity.GetLength() = %.10f", m_applyVelocity.GetLength());
        //AZ_Printf("", "m_applyVelocity.GetX() = %.10f", m_applyVelocity.GetX());
        //AZ_Printf("", "m_applyVelocity.GetY() = %.10f", m_applyVelocity.GetY());
        //AZ_Printf("", "m_applyVelocity.GetZ() = %.10f", m_applyVelocity.GetZ());
        //AZ_Printf("", "m_sprintIncrementTime = %.10f", m_sprintIncrementTime);
        //AZ_Printf("", "m_sprintValue = %.10f", m_sprintValue);
        //AZ_Printf("", "m_sprintAccelAdjust = %.10f", m_sprintAccelAdjust);
        //AZ_Printf("", "m_sprintVelocityAdjust = %.10f", m_sprintVelocityAdjust);
        //AZ_Printf("", "m_sprintHeldDuration = %.10f", m_sprintHeldDuration);
        //AZ_Printf("", "m_sprintDecrementPause = %.10f", m_sprintDecrementPause);
        //AZ_Printf("", "m_sprintPrevDecrementPause = %.10f", m_sprintPrevDecrementPause);
        //AZ_Printf("", "m_sprintCooldown = %.10f", m_sprintCooldown);
        //static float prevVelocity = m_applyVelocity.GetLength();
        //AZ_Printf("", "dv/dt = %.10f", (m_applyVelocity.GetLength() - prevVelocity)/deltaTime);
        //prevVelocity = m_applyVelocity.GetLength();
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "X Position = %.10f", pos.GetX());
        //AZ_Printf("", "Y Position = %.10f", pos.GetY());
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("","");
    }

    void FirstPersonControllerComponent::CheckGrounded(const float& deltaTime)
    {
        // Obtaining the PhysX Character Controller values here instead of the Activate method
        // because doing this inside the Activate method does not yield what is given to the PhysX component
        // even though PhysicsCharacterControllerService is set as a required service
        if(!m_obtainedPhysxCharacterValues)
        {
            // Obtain the PhysX Character Controller's capsule height and radius
            // and use those dimensions for the ground detection shapecast capsule
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleHeight, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);
            PhysX::CharacterControllerRequestBus::EventResult(m_capsuleRadius, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetRadius);
            Physics::CharacterRequestBus::EventResult(m_maxGroundedAngleDegrees, GetEntityId(),
                &Physics::CharacterRequestBus::Events::GetSlopeLimitDegrees);

            if(m_crouchDistance > m_capsuleHeight - 2.f*m_capsuleRadius)
                m_crouchDistance = m_capsuleHeight - 2.f*m_capsuleRadius;

            // Set the max grounded angle to be slightly greater than the PhysX Character Controller's
            // maximum slope angle value
            m_maxGroundedAngleDegrees += 0.01f;

            //AZ_Printf("", "m_capsuleHeight = %.10f", m_capsuleHeight);
            //AZ_Printf("", "m_capsuleRadius = %.10f", m_capsuleRadius);
            //AZ_Printf("", "m_maxGroundedAngleDegrees = %.10f", m_maxGroundedAngleDegrees);

            // Calculate the actual offset that will be used to translate the intersection capsule
            m_capsuleOffsetTranslation = m_capsuleHeight/2.f - m_capsuleOffset;
            m_capsuleJumpHoldOffsetTranslation = m_capsuleHeight/2.f - m_capsuleJumpHoldOffset;

            UpdateJumpTime();

            m_obtainedPhysxCharacterValues = true;
        }

        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Used to determine when event notifications occur
        const bool prevGrounded = m_grounded;
        const bool prevGroundClose = m_groundClose;

        // Rotate the pose by 90 degrees on the Y axis since by default the capsule's height
        // is oriented along the X axis when we want it oriented along the Z axis
        AZ::Transform capsuleIntersectionPose = AZ::Transform::CreateRotationY(AZ::Constants::HalfPi);
        // Move the capsule to the location of the character and apply the Z offset

        capsuleIntersectionPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleOffsetTranslation));

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
            m_capsuleRadius,
            m_capsuleHeight,
            capsuleIntersectionPose,
            AZ::Vector3(0.f, 0.f, -1.f),
            0.f,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            AzPhysics::CollisionGroup::All,
            nullptr);

        request.m_reportMultipleHits = true;

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        AZStd::vector<AZ::Vector3> steepNormals;

        // Disregard intersections with the character's collider, its child entities,
        // and if the slope angle of the thing that's intersecting is greater than the max grounded angle
        auto selfChildSlopeEntityCheck = [this, &steepNormals](AzPhysics::SceneQueryHit& hit)
            {
                if(hit.m_entityId == GetEntityId())
                    return true;

                // Obtain the child IDs if we don't already have them
                if(!m_obtainedChildIds)
                {
                    AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                    m_obtainedChildIds = true;
                }

                for(AZ::EntityId id: m_children)
                {
                    if(hit.m_entityId == id)
                        return true;
                }

                if(abs(hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ())) > m_maxGroundedAngleDegrees)
                {
                    steepNormals.push_back(hit.m_normal);
                    //AZ_Printf("", "Steep Angle EntityId = %s", hit.m_entityId.ToString().c_str());
                    //AZ_Printf("", "Steep Angle = %.10f", hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
                    return true;
                }

                return false;
            };

        AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
        m_grounded = hits ? true : false;

        // Check to see if the sum of the steep angles is less than or equal to m_maxGroundedAngleDegrees
        if(!m_grounded && steepNormals.size() > 1)
        {
            AZ::Vector3 sumNormals = AZ::Vector3::CreateZero();
            for(AZ::Vector3 normal: steepNormals)
                sumNormals += normal;

            //AZ_Printf("", "Sum of Steep Angles = %.10f", sumNormals.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
            if(abs(sumNormals.AngleSafeDeg(AZ::Vector3::CreateAxisZ())) <= m_maxGroundedAngleDegrees)
                m_grounded = true;
        }
        steepNormals.clear();

        if(m_scriptSetGroundTick)
        {
            m_grounded = m_scriptGrounded;
            m_scriptSetGroundTick = false;
        }

        if(m_grounded)
        {
            m_groundClose = true;
            m_airTime = 0.f;
        }
        // Check to see if the character is still close to the ground after pressing and holding the jump key
        // to allow them to jump higher based on the m_capsuleJumpHoldOffsetTranslation distance
        else
        {
            m_airTime += deltaTime;

            capsuleIntersectionPose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsuleJumpHoldOffsetTranslation));

            request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsuleRadius,
                m_capsuleHeight,
                capsuleIntersectionPose,
                AZ::Vector3(0.f, 0.f, -1.f),
                0.f,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                AzPhysics::CollisionGroup::All,
                nullptr);

            request.m_reportMultipleHits = true;

            hits = sceneInterface->QueryScene(sceneHandle, &request);
            AZStd::erase_if(hits.m_hits, selfChildSlopeEntityCheck);
            m_groundClose = hits ? true : false;

            if(m_scriptSetGroundCloseTick)
            {
                m_groundClose = m_scriptGroundClose;
                m_scriptSetGroundCloseTick = false;
            }
            //AZ_Printf("", "m_groundClose = %s", m_groundClose ? "true" : "false");
        }

        // Trigger an event notification if the player hits the ground, is about to hit the ground,
        // or just left the ground (via jumping or otherwise)
        if(!prevGrounded && m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundHit);
        else if(!prevGroundClose && m_groundClose)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundSoonHit);
        else if(prevGrounded && !m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnUngrounded);
    }

    void FirstPersonControllerComponent::UpdateJumpTime()
    {
        // Calculate the amount of time that the jump key can be held based on m_capsuleJumpHoldOffset
        // divided by the average of the initial jump velocity and the velocity at the edge of the capsule
        const float jumpVelocityCapsuleEdgeSquared = m_jumpInitialVelocity*m_jumpInitialVelocity
                                                         + 2.f*m_gravity*m_jumpHeldGravityFactor*m_capsuleJumpHoldOffset;
        // If the initial velocity is large enough such that the apogee can be reached outside of the capsule
        // then compute how long the jump key is held while still inside the jump hold offset intersection capsule
        if(jumpVelocityCapsuleEdgeSquared >= 0.f)
            m_jumpTime = m_capsuleJumpHoldOffset / ((m_jumpInitialVelocity
                                                        + sqrt(jumpVelocityCapsuleEdgeSquared)) / 2.f);
        // Otherwise the apogee will be reached inside m_capsuleJumpHoldOffset
        // and the jump time needs to computed accordingly
        else
            m_jumpTime = abs(m_jumpInitialVelocity / (m_gravity*m_jumpHeldGravityFactor));
    }

    void FirstPersonControllerComponent::UpdateVelocityZ(const float& deltaTime)
    {
        AZ::Vector3 currentVelocity = AZ::Vector3::CreateZero();
        Physics::CharacterRequestBus::EventResult(currentVelocity, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetVelocity);

        // Used for the Verlet integration averaging calculation
        m_zVelocityPrevDelta = m_zVelocityCurrentDelta;

        if(m_grounded && (m_jumpReqRepress || (currentVelocity.GetZ() <= 0.f && m_zVelocity <= 0.f)))
        {
            if(m_jumpValue && !m_jumpHeld)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                        m_crouching = false;
                    return;
                }
                m_zVelocityCurrentDelta = m_jumpInitialVelocity;
                m_jumpHeld = true;
                m_jumpReqRepress = false;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnFirstJump);
            }
            else
            {
                m_zVelocity = 0.f;
                m_zVelocityCurrentDelta = 0.f;
                m_jumpCounter = 0.f;

                if(m_jumpValue == 0.f && m_jumpHeld)
                    m_jumpHeld = false;

                if(m_doubleJumpEnabled && m_secondJump)
                    m_secondJump = false;
            }
        }
        else if(m_jumpCounter < m_jumpTime && currentVelocity.GetZ() > 0.f && m_jumpHeld && !m_jumpReqRepress)
        {
            if(m_jumpValue == 0.f)
            {
                m_jumpHeld = false;
                m_jumpCounter = 0.f;
                m_zVelocityCurrentDelta = m_gravity * deltaTime;
            }
            else
            {
                m_jumpCounter += deltaTime;
                m_zVelocityCurrentDelta = m_gravity * m_jumpHeldGravityFactor * deltaTime;
            }
        }
        else
        {
            if(!m_jumpReqRepress)
                m_jumpReqRepress = true;

            if(m_jumpCounter != 0.f)
                m_jumpCounter = 0.f;

            if(currentVelocity.GetZ() <= 0.f)
                m_zVelocityCurrentDelta = m_gravity * m_jumpFallingGravityFactor * deltaTime;
            else
                m_zVelocityCurrentDelta = m_gravity * deltaTime;

            if(!m_doubleJumpEnabled && !m_jumpHeld)
                m_jumpHeld = true;
            else if(m_doubleJumpEnabled && m_jumpValue == 0.f && m_jumpHeld)
                m_jumpHeld = false;

            if(m_doubleJumpEnabled && !m_secondJump && !m_jumpHeld && m_jumpValue != 0.f)
            {
                if(!m_standing)
                {
                    if(m_crouchJumpCausesStanding)
                        m_crouching = false;
                    return;
                }
                m_zVelocity = m_jumpInitialVelocity;
                m_zVelocityCurrentDelta = 0.f;
                m_secondJump = true;
                m_jumpHeld = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSecondJump);
            }
        }

        // Perform an average of the current and previous Z velocity delta
        // as described by Verlet integration, which should reduce accumulated error
        m_zVelocity += (m_zVelocityCurrentDelta + m_zVelocityPrevDelta) / 2.f;

        // Account for the case where the PhysX Character Gameplay component's gravity is used instead
        if(m_gravity == 0.f && m_grounded && currentVelocity.GetZ() < 0.f)
            m_zVelocity = 0.f;

        // Debug print statements to observe the jump mechanic
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("", "currentVelocity.GetZ() = %.10f", currentVelocity.GetZ());
        //AZ_Printf("", "m_zVelocityPrevDelta = %.10f", m_zVelocityPrevDelta);
        //AZ_Printf("", "m_zVelocityCurrentDelta = %.10f", m_zVelocityCurrentDelta);
        //AZ_Printf("", "m_zVelocity = %.10f", m_zVelocity);
        //AZ_Printf("", "m_grounded = %s", m_grounded ? "true" : "false");
        //AZ_Printf("", "m_jumpCounter = %.10f", m_jumpCounter);
        //AZ_Printf("", "deltaTime = %.10f", deltaTime);
        //AZ_Printf("", "m_jumpTime = %.10f", m_jumpTime);
        //AZ_Printf("", "m_capsuleJumpHoldOffset = %.10f", m_capsuleJumpHoldOffset);
        //static float prevZVelocity = m_zVelocity;
        //AZ_Printf("", "dvz/dt = %.10f", (m_zVelocity - prevZVelocity)/deltaTime);
        //AZ_Printf("","");
        //prevZVelocity = m_zVelocity;
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime)
    {
        UpdateRotation(deltaTime);

        CheckGrounded(deltaTime);

        if(m_grounded)
            CrouchManager(deltaTime);

        // So long as the character is grounded or depending on how the update X&Y velocity while jumping
        // boolean values are set, and based on the state of jumping/falling, update the X&Y velocity accordingly
        if(m_grounded || (m_updateXYAscending && m_updateXYDecending && !m_updateXYOnlyNearGround)
           || ((m_updateXYAscending && m_zVelocity >= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose))
           || ((m_updateXYDecending && m_zVelocity <= 0.f) && (!m_updateXYOnlyNearGround || m_groundClose)) )
            UpdateVelocityXY(deltaTime);

        UpdateVelocityZ(deltaTime);

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            (m_applyVelocity + AZ::Vector3::CreateAxisZ(m_zVelocity)));
    }

    // Event Notification methods for use in scripts
    void FirstPersonControllerComponent::OnGroundHit(){}
    void FirstPersonControllerComponent::OnGroundSoonHit(){}
    void FirstPersonControllerComponent::OnUngrounded(){}
    void FirstPersonControllerComponent::OnCrouched(){}
    void FirstPersonControllerComponent::OnStoodUp(){}
    void FirstPersonControllerComponent::OnFirstJump(){}
    void FirstPersonControllerComponent::OnSecondJump(){}
    void FirstPersonControllerComponent::OnSprintCooldown(){}

    // Request Bus getter and setter methods for use in scripts
    AZ::EntityId FirstPersonControllerComponent::GetActiveCameraId() const
    {
        return m_activeCameraEntity->GetId();
    }
    bool FirstPersonControllerComponent::GetGrounded() const
    {
        return m_grounded;
    }
    void FirstPersonControllerComponent::SetGroundedForTick(const bool& new_grounded)
    {
        m_scriptGrounded = new_grounded;
        m_scriptSetGroundTick = true;
    }
    bool FirstPersonControllerComponent::GetGroundClose() const
    {
        return m_groundClose;
    }
    void FirstPersonControllerComponent::SetGroundCloseForTick(const bool& new_groundClose)
    {
        m_scriptGroundClose = new_groundClose;
        m_scriptSetGroundCloseTick = true;
    }
    float FirstPersonControllerComponent::GetAirTime() const
    {
        return m_airTime;
    }
    float FirstPersonControllerComponent::GetJumpKeyValue() const
    {
        return m_jumpValue;
    }
    float FirstPersonControllerComponent::GetGravity() const
    {
        return m_gravity;
    }
    void FirstPersonControllerComponent::SetGravity(const float& new_gravity)
    {
        m_gravity = new_gravity;
        UpdateJumpTime();
    }
    float FirstPersonControllerComponent::GetInitialJumpVelocity() const
    {
        return m_jumpInitialVelocity;
    }
    void FirstPersonControllerComponent::SetInitialJumpVelocity(const float& new_jumpInitialVelocity)
    {
        m_jumpInitialVelocity = new_jumpInitialVelocity;
        UpdateJumpTime();
    }
    bool FirstPersonControllerComponent::GetDoubleJump() const
    {
        return m_doubleJumpEnabled;
    }
    void FirstPersonControllerComponent::SetDoubleJump(const bool& new_doubleJumpEnabled)
    {
        m_doubleJumpEnabled = new_doubleJumpEnabled;
    }
    float FirstPersonControllerComponent::GetCapsuleOffset() const
    {
        return m_capsuleOffset;
    }
    void FirstPersonControllerComponent::SetCapsuleOffset(const float& new_capsuleOffset)
    {
        m_capsuleOffset = new_capsuleOffset;
        UpdateJumpTime();
    }
    float FirstPersonControllerComponent::GetCapsuleJumpHoldOffset() const
    {
        return m_capsuleJumpHoldOffset;
    }
    void FirstPersonControllerComponent::SetCapsuleJumpHoldOffset(const float& new_capsuleJumpHoldOffset)
    {
        m_capsuleJumpHoldOffset = new_capsuleJumpHoldOffset;
        UpdateJumpTime();
    }
    float FirstPersonControllerComponent::GetMaxGroundedAngleDegrees() const
    {
        return m_maxGroundedAngleDegrees;
    }
    void FirstPersonControllerComponent::SetMaxGroundedAngleDegrees(const float& new_maxGroundedAngleDegrees)
    {
        m_maxGroundedAngleDegrees = new_maxGroundedAngleDegrees;
    }
    float FirstPersonControllerComponent::GetTopWalkSpeed() const
    {
        return m_speed;
    }
    void FirstPersonControllerComponent::SetTopWalkSpeed(const float& new_speed)
    {
        m_speed = new_speed;
    }
    float FirstPersonControllerComponent::GetWalkAcceleration() const
    {
        return m_accel;
    }
    void FirstPersonControllerComponent::SetWalkAcceleration(const float& new_accel)
    {
        m_accel = new_accel;
    }
    float FirstPersonControllerComponent::GetWalkDeceleration() const
    {
        return m_decel;
    }
    void FirstPersonControllerComponent::SetWalkDeceleration(const float& new_decel)
    {
        m_decel = new_decel;
    }
    float FirstPersonControllerComponent::GetWalkBreak() const
    {
        return m_break;
    }
    void FirstPersonControllerComponent::SetWalkBreak(const float& new_break)
    {
        m_break = new_break;
    }
    float FirstPersonControllerComponent::GetSprintScale() const
    {
        return m_sprintScale;
    }
    void FirstPersonControllerComponent::SetSprintScale(const float& new_sprintScale)
    {
        m_sprintScale = new_sprintScale;
    }
    float FirstPersonControllerComponent::GetCrouchScale() const
    {
        return m_crouchScale;
    }
    void FirstPersonControllerComponent::SetCrouchScale(const float& new_crouchScale)
    {
        m_crouchScale = new_crouchScale;
    }
    float FirstPersonControllerComponent::GetSprintHeldTime() const
    {
        return m_sprintHeldDuration;
    }
    void FirstPersonControllerComponent::SetSprintHeldTime(const float& new_sprintHeldDuration)
    {
        m_sprintHeldDuration = new_sprintHeldDuration;
    }
    float FirstPersonControllerComponent::GetSprintCooldown() const
    {
        return m_sprintCooldown;
    }
    void FirstPersonControllerComponent::SetSprintCooldown(const float& new_sprintCooldown)
    {
        m_sprintCooldown = new_sprintCooldown;
    }
    float FirstPersonControllerComponent::GetSprintPauseTime() const
    {
        return m_sprintDecrementPause;
    }
    void FirstPersonControllerComponent::SetSprintPauseTime(const float& new_sprintDecrementPause)
    {
        m_sprintDecrementPause = new_sprintDecrementPause;
    }
    bool FirstPersonControllerComponent::GetCrouching() const
    {
        return m_crouching;
    }
    void FirstPersonControllerComponent::SetCrouching(const bool& new_crouching)
    {
        m_crouching = new_crouching;
    }
    float FirstPersonControllerComponent::GetCrouchDistance() const
    {
        return m_crouchDistance;
    }
    void FirstPersonControllerComponent::SetCrouchDistance(const float& new_crouchDistance)
    {
        m_crouchDistance = new_crouchDistance;
    }
    float FirstPersonControllerComponent::GetCrouchTime() const
    {
        return m_crouchTime;
    }
    void FirstPersonControllerComponent::SetCrouchTime(const float& new_crouchTime)
    {
        m_crouchTime = new_crouchTime;
    }
    bool FirstPersonControllerComponent::GetCrouchEnableToggle() const
    {
        return m_crouchEnableToggle;
    }
    void FirstPersonControllerComponent::SetCrouchEnableToggle(const bool& new_crouchEnableToggle)
    {
        m_crouchEnableToggle = new_crouchEnableToggle;
    }
    bool FirstPersonControllerComponent::GetCrouchJumpCausesStanding() const
    {
        return m_crouchJumpCausesStanding;
    }
    void FirstPersonControllerComponent::SetCrouchJumpCausesStanding(const bool& new_crouchJumpCausesStanding)
    {
        m_crouchJumpCausesStanding = new_crouchJumpCausesStanding;
    }
    bool FirstPersonControllerComponent::GetCrouchSprintCausesStanding() const
    {
        return m_crouchSprintCausesStanding;
    }
    void FirstPersonControllerComponent::SetCrouchSprintCausesStanding(const bool& new_crouchSprintCausesStanding)
    {
        m_crouchSprintCausesStanding = new_crouchSprintCausesStanding;
    }
    bool FirstPersonControllerComponent::GetCrouchPriorityWhenSprintPressed() const
    {
        return m_crouchPriorityWhenSprintPressed;
    }
    void FirstPersonControllerComponent::SetCrouchPriorityWhenSprintPressed(const bool& new_crouchPriorityWhenSprintPressed)
    {
        m_crouchPriorityWhenSprintPressed = new_crouchPriorityWhenSprintPressed;
    }
    float FirstPersonControllerComponent::GetCameraPitchSensitivity() const
    {
        return m_pitchSensitivity;
    }
    void FirstPersonControllerComponent::SetCameraPitchSensitivity(const float& new_pitchSensitivity)
    {
        m_pitchSensitivity = new_pitchSensitivity;
    }
    float FirstPersonControllerComponent::GetCameraYawSensitivity() const
    {
        return m_yawSensitivity;
    }
    void FirstPersonControllerComponent::SetCameraYawSensitivity(const float& new_yawSensitivity)
    {
        m_yawSensitivity = new_yawSensitivity;
    }
    float FirstPersonControllerComponent::GetCameraRotationDampFactor() const
    {
        return m_rotationDamp;
    }
    void FirstPersonControllerComponent::SetCameraRotationDampFactor(const float& new_rotationDamp)
    {
        m_rotationDamp = new_rotationDamp;
    }
    void FirstPersonControllerComponent::UpdateCameraPitch(const float& new_cameraPitchAngle)
    {
        m_cameraRotationAngles[0] = new_cameraPitchAngle - m_pitchValue * m_pitchSensitivity;
        m_rotatingPitchViaScriptGamepad = true;
    }
    void FirstPersonControllerComponent::UpdateCameraYaw(const float& new_cameraYawAngle)
    {
        m_cameraRotationAngles[2] = new_cameraYawAngle - m_yawValue * m_yawSensitivity;
        m_rotatingYawViaScriptGamepad = true;
    }
    float FirstPersonControllerComponent::GetHeading() const
    {
        return m_currentHeading;
    }
}
