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
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_str_yaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_str_pitch)
              ->Field("Forward Key", &FirstPersonControllerComponent::m_str_forward)
              ->Field("Back Key", &FirstPersonControllerComponent::m_str_back)
              ->Field("Left Key", &FirstPersonControllerComponent::m_str_left)
              ->Field("Right Key", &FirstPersonControllerComponent::m_str_right)
              ->Field("Sprint Key", &FirstPersonControllerComponent::m_str_sprint)
              ->Field("Crouch Key", &FirstPersonControllerComponent::m_str_crouch)
              ->Field("Jump Key", &FirstPersonControllerComponent::m_str_jump)

              // Camera Rotation group
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yaw_sensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitch_sensitivity)
              ->Field("Camera Rotation Damp Factor", &FirstPersonControllerComponent::m_rotation_damp)

              // Scale Factors group
              ->Field("Forward Scale", &FirstPersonControllerComponent::m_forward_scale)
              ->Field("Back Scale", &FirstPersonControllerComponent::m_back_scale)
              ->Field("Left Scale", &FirstPersonControllerComponent::m_left_scale)
              ->Field("Right Scale", &FirstPersonControllerComponent::m_right_scale)
              ->Field("Sprint Scale", &FirstPersonControllerComponent::m_sprint_scale)
              ->Field("Crouch Scale", &FirstPersonControllerComponent::m_crouch_scale)

              // X&Y Movement group
              ->Field("Top Walking Speed (m/s)", &FirstPersonControllerComponent::m_speed)
              ->Field("Walking Acceleration (m/s²)", &FirstPersonControllerComponent::m_accel)
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Breaking Factor", &FirstPersonControllerComponent::m_break)

              // Sprint Timing group
              ->Field("Sprint Max Time (sec)", &FirstPersonControllerComponent::m_sprint_max_time)
              ->Field("Sprint Cooldown (sec)", &FirstPersonControllerComponent::m_sprint_cooldown_time)

              // Crouching group
              ->Field("Crouch Camera Distance", &FirstPersonControllerComponent::m_crouch_camera_distance)
              ->Field("Crouch Camera Time", &FirstPersonControllerComponent::m_crouch_camera_time)
              ->Field("Crouch Enable Toggle", &FirstPersonControllerComponent::m_crouch_enable_toggle)

              // Jumping group
              ->Field("Gravity (m/s²)", &FirstPersonControllerComponent::m_gravity)
              ->Field("Jump Initial Velocity (m/s)", &FirstPersonControllerComponent::m_jump_initial_velocity)
              ->Field("Jump Held Gravity Factor", &FirstPersonControllerComponent::m_jump_held_gravity_factor)
              ->Field("Jump Falling Gravity Factor", &FirstPersonControllerComponent::m_jump_falling_gravity_factor)
              ->Field("XY Acceleration Jump Factor (m/s²)", &FirstPersonControllerComponent::m_jump_accel_factor)
              ->Field("Capsule Grounded Offset (m)", &FirstPersonControllerComponent::m_capsule_offset)
              ->Field("Capsule Jump Hold Offset (m)", &FirstPersonControllerComponent::m_capsule_jump_hold_offset)
              ->Field("Enable Double Jump", &FirstPersonControllerComponent::m_double_jump_enabled)
              ->Field("Update X&Y Velocity When Ascending", &FirstPersonControllerComponent::m_update_xy_ascending)
              ->Field("Update X&Y Velocity When Decending", &FirstPersonControllerComponent::m_update_xy_descending)
              ->Field("Update X&Y Velocity Only When Ground Close", &FirstPersonControllerComponent::m_update_xy_only_near_ground)

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
                        &FirstPersonControllerComponent::m_str_yaw,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_pitch,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_forward,
                        "Forward Key", "Key for moving forward")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_back,
                        "Back Key", "Key for moving back")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_left,
                        "Left Key", "Key for moving left")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_right,
                        "Right Key", "Key for moving right")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_sprint,
                        "Sprint Key", "Key for sprinting")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_crouch,
                        "Crouch Key", "Key for crouching")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_jump,
                        "Jump Key", "Key for jumping")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Camera Rotation")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yaw_sensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitch_sensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rotation_damp,
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
                        &FirstPersonControllerComponent::m_forward_scale,
                        "Forward Scale", "Forward movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_back_scale,
                        "Back Scale", "Back movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_left_scale,
                        "Left Scale", "Left movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_right_scale,
                        "Right Scale", "Right movement scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_scale,
                        "Sprint Scale", "Sprint scale factor")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouch_scale,
                        "Crouch Scale", "Crouch scale factor")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Sprint Timing")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_max_time,
                        "Sprint Max Time (sec)", "The maximum consecutive sprinting time")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_cooldown_time,
                        "Sprint Cooldown (sec)", "The time required to wait before sprinting again when the maximum consecutive sprint time has been reached")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouching")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouch_camera_distance,
                        "Crouch Camera Distance", "Determines the distance the camera will move on the Z axis")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouch_camera_time,
                        "Crouch Camera Time", "Determines the time it takes the camera to complete its movement")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_crouch_enable_toggle,
                        "Crouch Enable Toggle", "Determines whether the crouch key toggles crouching")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Jumping")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_gravity,
                        "Gravity (m/s²)", "Z Acceleration due to gravity, set this to 0 if you prefer to use the PhysX Character Gameplay component's gravity instead")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jump_initial_velocity,
                        "Jump Initial Velocity (m/s)", "Initial jump velocity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jump_held_gravity_factor,
                        "Jump Held Gravity Factor", "The factor applied to the character's gravity for the beginning of the jump")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jump_falling_gravity_factor,
                        "Jump Falling Gravity Factor", "The factor applied to the character's gravity when the Z velocity is negative")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_jump_accel_factor,
                        "XY Acceleration Jump Factor (m/s²)", "X & Y acceleration factor while jumping but still close to the ground")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_offset,
                        "Capsule Grounded Offset (m)", "The capsule's ground detect offset in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_jump_hold_offset,
                        "Capsule Jump Hold Offset (m)", "The capsule's jump hold offset in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_double_jump_enabled,
                        "Enable Double Jump", "Turn this on if you want to enable double jumping")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_update_xy_ascending,
                        "Update X&Y Velocity When Ascending", "Determines if the X&Y velocity components will be updated when ascending")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_update_xy_descending,
                        "Update X&Y Velocity When Descending", "Determines if the X&Y velocity components will be updated when descending")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_update_xy_only_near_ground,
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
                ->Event("Get Sprint Held Time", &FirstPersonControllerComponentRequests::GetSprintHeldTime)
                ->Event("Set Sprint Held Time", &FirstPersonControllerComponentRequests::SetSprintHeldTime)
                ->Event("Get Sprint Cooldown", &FirstPersonControllerComponentRequests::GetSprintCooldown)
                ->Event("Set Sprint Cooldown", &FirstPersonControllerComponentRequests::SetSprintCooldown)
                ->Event("Get Sprint Pause Time", &FirstPersonControllerComponentRequests::GetSprintPauseTime)
                ->Event("Set Sprint Pause Time", &FirstPersonControllerComponentRequests::SetSprintPauseTime)
                ->Event("Get Crouching", &FirstPersonControllerComponentRequests::GetCrouching)
                ->Event("Set Crouching", &FirstPersonControllerComponentRequests::SetCrouching)
                ->Event("Get Crouch Camera Distance", &FirstPersonControllerComponentRequests::GetCrouchCameraDistance)
                ->Event("Set Crouch Camera Distance", &FirstPersonControllerComponentRequests::SetCrouchCameraDistance)
                ->Event("Get Crouch Camera Time", &FirstPersonControllerComponentRequests::GetCrouchCameraTime)
                ->Event("Set Crouch Camera Time", &FirstPersonControllerComponentRequests::SetCrouchCameraTime)
                ->Event("Get Crouch Enable Toggle", &FirstPersonControllerComponentRequests::GetCrouchEnableToggle)
                ->Event("Set Crouch Enable Toggle", &FirstPersonControllerComponentRequests::SetCrouchEnableToggle)
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

        // Obtain the PhysX Character Controller's capsule height and radius
        // and use those dimensions for the ground detection shapecast capsule
        PhysX::CharacterControllerRequestBus::EventResult(m_capsule_height, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetHeight);
        PhysX::CharacterControllerRequestBus::EventResult(m_capsule_radius, GetEntityId(),
            &PhysX::CharacterControllerRequestBus::Events::GetRadius);
        Physics::CharacterRequestBus::EventResult(m_max_grounded_angle_degrees, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetSlopeLimitDegrees);

        // Set the max grounded angle to be slightly greater than the PhysX Character Controller's
        // maximum slope angle value
        m_max_grounded_angle_degrees += 0.01f;

        // Calculate the actual offset that will be used to translate the intersection capsule
        m_capsule_offset_translation = m_capsule_height/2.f - m_capsule_offset;
        m_capsule_jump_hold_offset_translation = m_capsule_height/2.f - m_capsule_jump_hold_offset;

        if(m_control_map.size() != (sizeof(m_input_names) / sizeof(AZStd::string*)))
        {
            AZ_Printf("FirstPersonControllerComponent",
                      "ERROR: Number of input IDs not equal to number of input names!");
        }
        else
        {
            for(auto& it_event : m_control_map)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_input_names[std::distance(m_control_map.begin(), m_control_map.find(it_event.first))])->c_str());
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

    void FirstPersonControllerComponent::GetRequredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
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
            m_sprint_value = m_sprint_pressed_value = value * m_sprint_scale;
        }

        for(auto& it_event : m_control_map)
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

        for(auto& it_event : m_control_map)
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
            m_yaw_value = value;
        }
        else if(*inputId == m_RotatePitchEventId)
        {
            m_pitch_value = value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if(*inputId == m_SprintEventId)
        {
            m_sprint_value = m_sprint_pressed_value = value * m_sprint_scale;
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
            m_right_value = inputChannel.GetValue();
            m_left_value = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LL)
        {
            m_right_value = 0.f;
            m_left_value = -1.f*inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LU)
        {
            m_forward_value = inputChannel.GetValue();
            m_back_value = 0.f;
        }
        else if(channelId == AzFramework::InputDeviceGamepad::ThumbStickDirection::LD)
        {
            m_forward_value = 0.f;
            m_back_value = -1.f*inputChannel.GetValue();
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RY)
        {
            m_camera_rotation_angles[0] = inputChannel.GetValue() * m_pitch_sensitivity;
            m_rotating_pitch_via_script_gamepad = true;
        }

        if(channelId == AzFramework::InputDeviceGamepad::ThumbStickAxis1D::RX)
        {
            m_camera_rotation_angles[2] = -1.f*inputChannel.GetValue() * m_yaw_sensitivity;
            m_rotating_yaw_via_script_gamepad = true;
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
        if(!m_rotating_pitch_via_script_gamepad)
            m_camera_rotation_angles[0] = -1.f * m_pitch_value * m_pitch_sensitivity;
        else
            m_rotating_pitch_via_script_gamepad = false;

        // Multiply by -1 since moving the mouse to the right produces a positive value
        // but a positive rotation about Z is counterclockwise
        if(!m_rotating_yaw_via_script_gamepad)
            m_camera_rotation_angles[2] = -1.f * m_yaw_value * m_yaw_sensitivity;
        else
            m_rotating_yaw_via_script_gamepad = false;

        const AZ::Quaternion target_look_rotation_delta = AZ::Quaternion::CreateFromEulerAnglesRadians(
            AZ::Vector3::CreateFromFloat3(m_camera_rotation_angles));

        if(m_rotation_damp*deltaTime <= 1.f)
            m_new_look_rotation_delta = m_new_look_rotation_delta.Slerp(target_look_rotation_delta, m_rotation_damp*deltaTime);
        else
            m_new_look_rotation_delta = target_look_rotation_delta;
    }

    void FirstPersonControllerComponent::UpdateRotation(const float& deltaTime)
    {
        AZ::TransformInterface* t = GetEntity()->GetTransform();

        SlerpRotation(deltaTime);
        const AZ::Vector3 new_look_rotation_delta = m_new_look_rotation_delta.GetEulerRadians();

        t->RotateAroundLocalZ(new_look_rotation_delta.GetZ());

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        float current_pitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(current_pitch) <= HalfPi ||
           current_pitch >= HalfPi && new_look_rotation_delta.GetX() < 0.f ||
           current_pitch <= -HalfPi && new_look_rotation_delta.GetX() > 0.f)
        {
            t->RotateAroundLocalX(new_look_rotation_delta.GetX());
            current_pitch = t->GetLocalRotation().GetX();
        }
        if(abs(current_pitch) > HalfPi)
        {
            if(current_pitch > HalfPi)
                t->RotateAroundLocalX(HalfPi - current_pitch);
            else
                t->RotateAroundLocalX(-HalfPi - current_pitch);
        }

        m_current_heading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();
    }

    AZ::Vector3 FirstPersonControllerComponent::LerpVelocity(const AZ::Vector3& target_velocity, const float& deltaTime)
    {
        float total_lerp_time = m_last_applied_velocity.GetDistance(target_velocity)/m_accel;

        // Apply the sprint factor to the acceleration (dt) based on the sprint having been (recently) pressed
        const float last_lerp_time = m_lerp_time;

        float lerp_deltaTime = m_sprint_increment_time > 0.f ? deltaTime * (1.f + (m_sprint_pressed_value-1.f) * m_sprint_accel_adjust) : deltaTime;
        lerp_deltaTime *= m_grounded ? 1.f : m_jump_accel_factor;

        m_lerp_time += lerp_deltaTime;

        if(m_lerp_time >= total_lerp_time)
            m_lerp_time = total_lerp_time;

        // Lerp the velocity from the last applied velocity to the target velocity
        AZ::Vector3 new_velocity = m_last_applied_velocity.Lerp(target_velocity, m_lerp_time / total_lerp_time);

        // Decelerate at a different rate than the acceleration
        if(new_velocity.GetLength() < m_apply_velocity.GetLength())
        {
            // Get the current velocity vector with respect to the world coordinates
            const AZ::Vector3 apply_velocity_world = AZ::Quaternion::CreateRotationZ(-m_current_heading).TransformVector(m_apply_velocity);

            float deceleration_factor = m_decel;

            // Compare the direction of the current velocity vector against the desired direction
            // and if it's greater than 90 degrees then decelerate even more
            if(target_velocity.GetLength() != 0.f && abs(apply_velocity_world.Angle(target_velocity)) > AZ::Constants::HalfPi)
                deceleration_factor *= m_break;

            // Use the deceleration factor to get the lerp time closer to the total lerp time at a faster rate
            m_lerp_time = last_lerp_time + lerp_deltaTime * deceleration_factor;

            if(m_lerp_time >= total_lerp_time)
                m_lerp_time = total_lerp_time;

            new_velocity = m_last_applied_velocity.Lerp(target_velocity, m_lerp_time / total_lerp_time);
        }

        return new_velocity;
    }

    void FirstPersonControllerComponent::SprintManager(const AZ::Vector3& target_velocity, const float& deltaTime)
    {
        // The sprint value should never be 0 and it shouldn't be applied if you're trying to moving backwards
        if(m_sprint_value == 0.f
           || m_crouching
           || (!m_apply_velocity.GetY() && !m_apply_velocity.GetX())
           || (m_sprint_value != 1.f
               && ((!m_forward_value && !m_left_value && !m_right_value) ||
                   (!m_forward_value && -m_left_value == m_right_value) ||
                   (target_velocity.GetY() < 0.f)) ))
            m_sprint_value = 1.f;

        // Set the sprint value to 1 and reset the counter if there is no movement
        if(!m_apply_velocity.GetY() && !m_apply_velocity.GetX())
        {
            m_sprint_value = 1.f;
            m_sprint_increment_time = 0.f;
        }

        const float total_sprint_time = ((m_sprint_value-1.f)*m_speed)/m_accel;

        // If the sprint key is pressed then increment the sprint counter
        if(m_sprint_value != 1.f && m_sprint_held_duration < m_sprint_max_time && m_sprint_cooldown == 0.f)
        {
            // Sprint adjustment factor based on the angle of the target velocity
            // with respect to their frame of reference
            m_sprint_velocity_adjust = 1.f - target_velocity.Angle(AZ::Vector3::CreateAxisY())/(AZ::Constants::HalfPi);
            m_sprint_accel_adjust = m_sprint_velocity_adjust;

            m_sprint_increment_time += deltaTime;
            m_sprint_held_duration += deltaTime * m_sprint_velocity_adjust;

            m_sprint_decrementing = false;

            if(m_sprint_increment_time > total_sprint_time)
                m_sprint_increment_time = total_sprint_time;
        }
        // Otherwise if the sprint key isn't pressed then decrement the sprint counter
        else if(m_sprint_value == 1.f || m_sprint_held_duration >= m_sprint_max_time || m_sprint_cooldown != 0.f)
        {
            // Set the sprint velocity adjust to 0
            m_sprint_velocity_adjust = 0.f;

            m_sprint_increment_time -= deltaTime;
            if(m_sprint_increment_time < 0.f)
                m_sprint_increment_time = 0.f;

            // When the sprint held duration exceeds the maximum sprint time then initiate the cooldown period
            if(m_sprint_held_duration >= m_sprint_max_time && m_sprint_cooldown == 0.f)
            {
                m_sprint_held_duration = 0.f;
                m_sprint_cooldown = m_sprint_cooldown_time;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSprintCooldown);
            }
            else if(m_sprint_cooldown != 0.f)
            {
                m_sprint_cooldown -= deltaTime;
                if(m_sprint_cooldown < 0.f)
                    m_sprint_cooldown = 0.f;
            }
            // If the sprint cooldown time is longer than the maximum sprint duration
            // then apply a pause based on the difference between the two, times the ratio of
            // how long sprint was held divided by the maximum sprint duration prior to
            // decrementing the sprint held duration
            else if(m_sprint_cooldown_time > m_sprint_max_time)
            {
                m_sprint_decrement_pause -= deltaTime;

                if(m_sprint_held_duration > 0.f && !m_sprint_decrementing)
                {
                    m_sprint_decrement_pause = (m_sprint_cooldown_time - m_sprint_max_time)
                                                *(m_sprint_held_duration/m_sprint_max_time);
                    // m_sprint_prev_decrement_pause is not used here, but setting it for potential future use
                    m_sprint_prev_decrement_pause = m_sprint_decrement_pause;
                    m_sprint_decrementing = true;
                }

                if(m_sprint_decrement_pause <= 0.f)
                {
                    m_sprint_held_duration -= deltaTime;
                    m_sprint_decrement_pause = 0.f;
                    if(m_sprint_held_duration <= 0.f)
                    {
                        m_sprint_held_duration = 0.f;
                        m_sprint_decrementing = false;
                    }
                }
            }
            else if(m_sprint_cooldown_time <= m_sprint_max_time)
            {
                m_sprint_decrement_pause -= deltaTime;
                if(m_sprint_held_duration > 0.f && !m_sprint_decrementing)
                {
                    // Making the m_sprint_decrement_pause a factor of 0.1 here is somewhat arbitrary,
                    // this can be set to any other desired number if you have
                    // m_sprint_cooldown_time <= m_sprint_max_time.
                    // The decrement time here is also set based on the cooldown time and the ratio of the
                    // held duration divided by the maximum consecutive sprint time so that the pause is longer
                    // if you recently sprinted for a while.
                    m_sprint_decrement_pause = 0.1f * m_sprint_cooldown_time * m_sprint_held_duration / m_sprint_max_time;
                    m_sprint_prev_decrement_pause = m_sprint_decrement_pause;
                    m_sprint_decrementing = true;
                }

                if(m_sprint_decrement_pause <= 0.f)
                {
                    // Decrement the held duration by a factor of the ratio of the max sprint time plus the
                    // previous decrement pause time, divided by the cooldown time
                    // so there is not an incentive to elapse it.
                    // This makes it so that the held duration decrements and gets back to 0 at the same rate
                    // as if you were to just allow it to elapse.
                    m_sprint_held_duration -= deltaTime * ((m_sprint_max_time+m_sprint_prev_decrement_pause)/m_sprint_cooldown_time);
                    m_sprint_decrement_pause = 0.f;
                    if(m_sprint_held_duration <= 0.f)
                    {
                        m_sprint_held_duration = 0.f;
                        m_sprint_decrementing = false;
                    }
                }
            }
        }
    }

    void FirstPersonControllerComponent::CrouchManager(const float& deltaTime)
    {
        AZ::TransformInterface* camera_transform = m_activeCameraEntity->GetTransform();

        if(m_crouch_enable_toggle && m_crouch_prev_value == 0.f && m_crouch_value == 1.f)
        {
            m_crouching = !m_crouching;
        }
        else if(!m_crouch_enable_toggle)
        {
            if(m_crouch_value != 0.f)
                m_crouching = true;
            else
                m_crouching = false;
        }

        //AZ_Printf("", "m_crouching = %s", m_crouching ? "true" : "false");

        if(m_crouching && m_camera_local_z_travel_distance > -1.f * m_crouch_camera_distance)
        {
            float camera_travel_delta = -1.f * m_crouch_camera_distance * deltaTime / m_crouch_camera_time;
            m_camera_local_z_travel_distance += camera_travel_delta;

            if(m_camera_local_z_travel_distance <= -1.f * m_crouch_camera_distance)
            {
                camera_travel_delta += abs(m_camera_local_z_travel_distance) - m_crouch_camera_distance;
                m_camera_local_z_travel_distance = -1.f * m_crouch_camera_distance;
            }

            // Adjust the height of the collider capsule based on the crouching height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsule_height, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Subtract the distance to get down to the crouching height
            m_capsule_height += camera_travel_delta;
            //AZ_Printf("", "Crouching capsule height = %.10f", m_capsule_height);

            // Recalculate the ground detect shapecast capsule's offset
            m_capsule_offset_translation = m_capsule_height/2.f - m_capsule_offset;
            m_capsule_jump_hold_offset_translation = m_capsule_height/2.f - m_capsule_jump_hold_offset;

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsule_height);

            camera_transform->SetLocalZ(camera_transform->GetLocalZ() + camera_travel_delta);
        }
        else if(!m_crouching && m_camera_local_z_travel_distance != 0.f)
        {
            // Create a shapecast capsule that will be used to detect whether there is an obstruction
            // above the players head, and prevent them from fully standing up if there is
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

            // Rotate the pose by 90 degrees on the Y axis since by default the capsule's height
            // is oriented along the X axis when we want it oriented along the Z axis
            AZ::Transform capsule_intersect_pose = AZ::Transform::CreateRotationY(AZ::Constants::HalfPi);
            // Move the capsule to the location of the character and apply the Z offset

            capsule_intersect_pose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsule_height/2.f + m_capsule_offset));

            AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsule_radius,
                m_capsule_height,
                capsule_intersect_pose,
                AZ::Vector3(0.f, 0.f, 1.f),
                0.f,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                AzPhysics::CollisionGroup::All,
                nullptr);

            AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

            // Disregard intersections with the character's collider and its child entities,
            auto self_child_entity_check = [this](AzPhysics::SceneQueryHit& hit)
                {
                    if(hit.m_entityId == GetEntityId())
                        return true;

                    // Obtain the child IDs if we don't already have them
                    if(!m_obtained_child_ids)
                    {
                        AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                        m_obtained_child_ids = true;
                    }

                    for(AZ::EntityId id: m_children)
                    {
                        if(hit.m_entityId == id)
                            return true;
                    }

                    return false;
                };

            AZStd::erase_if(hits.m_hits, self_child_entity_check);

            // Bail if something is detected above the player
            if(hits)
                return;

            float camera_travel_delta = m_crouch_camera_distance * deltaTime / m_crouch_camera_time;
            m_camera_local_z_travel_distance += camera_travel_delta;

            if(m_camera_local_z_travel_distance >= 0.f)
            {
                camera_travel_delta -= m_camera_local_z_travel_distance;
                m_camera_local_z_travel_distance = 0.f;
            }

            // Adjust the height of the collider capsule based on the standing height
            PhysX::CharacterControllerRequestBus::EventResult(m_capsule_height, GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::GetHeight);

            // Add the distance to get back to the standing height
            m_capsule_height += camera_travel_delta;
            //AZ_Printf("", "Standing capsule height = %.10f", m_capsule_height);

            // Recalculate the ground detect shapecast capsule's offset
            m_capsule_offset_translation = m_capsule_height/2.f - m_capsule_offset;
            m_capsule_jump_hold_offset_translation = m_capsule_height/2.f - m_capsule_jump_hold_offset;

            PhysX::CharacterControllerRequestBus::Event(GetEntityId(),
                &PhysX::CharacterControllerRequestBus::Events::Resize, m_capsule_height);

            camera_transform->SetLocalZ(camera_transform->GetLocalZ() + camera_travel_delta);
        }

        m_crouch_prev_value = m_crouch_value;
    }

    void FirstPersonControllerComponent::UpdateVelocityXY(const float& deltaTime)
    {
        float forwardBack = m_forward_value * m_forward_scale + m_back_value * m_back_scale;
        float leftRight = m_left_value * m_left_scale + m_right_value * m_right_scale;

        // Remove the scale factor since it's going to be applied after the normalization
        if(forwardBack >= 0.f)
            forwardBack /= m_forward_scale;
        else
            forwardBack /= m_back_scale;

        if(leftRight >= 0.f)
            leftRight /= m_right_scale;
        else
            leftRight /= m_left_scale;

        AZ::Vector3 target_velocity = AZ::Vector3(leftRight, forwardBack, 0.f);

        // Normalize the vector if its magnitude is greater than 1 and then scale it
        if((forwardBack || leftRight) && sqrt(forwardBack*forwardBack + leftRight*leftRight) > 1.f)
            target_velocity.Normalize();

        if(target_velocity.GetY() >= 0.f)
            target_velocity.SetY(target_velocity.GetY() * m_forward_scale);
        else
            target_velocity.SetY(target_velocity.GetY() * m_back_scale);

        if(target_velocity.GetX() >= 0.f)
            target_velocity.SetX(target_velocity.GetX() * m_right_scale);
        else
            target_velocity.SetX(target_velocity.GetX() * m_left_scale);

        // Call the sprint manager
        SprintManager(target_velocity, deltaTime);

        // Apply the speed and sprint factor
        if(!m_crouching)
            target_velocity *= m_speed * (1.f + (m_sprint_value-1.f) * m_sprint_velocity_adjust);
        // Don't apply the sprint factor when crouching
        else
            target_velocity *= m_speed * m_crouch_scale;

        // Obtain the last applied velocity if the target velocity changed
        if(m_prev_target_velocity != target_velocity)
        {
            // Set the previous target velocity to the new one
            m_prev_target_velocity = target_velocity;

            // Store the last applied velocity to be used for the lerping
            m_last_applied_velocity = AZ::Quaternion::CreateRotationZ(-m_current_heading).TransformVector(m_apply_velocity);

            // Reset the lerp time since the target velocity changed
            m_lerp_time = 0.f;
        }

        // Rotate the target velocity vector so that it can be compared against the applied velocity
        const AZ::Vector3 target_velocity_world = AZ::Quaternion::CreateRotationZ(m_current_heading).TransformVector(target_velocity);

        // Lerp to the velocity if we're not already there
        if(m_apply_velocity != target_velocity_world)
            m_apply_velocity = AZ::Quaternion::CreateRotationZ(m_current_heading).TransformVector(LerpVelocity(target_velocity, deltaTime));

        // Debug print statements to observe the velocity, acceleration, and position
        //AZ_Printf("", "m_current_heading = %.10f", m_current_heading);
        //AZ_Printf("", "atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()) = %.10f", atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()));
        //AZ_Printf("", "m_apply_velocity.GetLength() = %.10f", m_apply_velocity.GetLength());
        //AZ_Printf("", "m_apply_velocity.GetX() = %.10f", m_apply_velocity.GetX());
        //AZ_Printf("", "m_apply_velocity.GetY() = %.10f", m_apply_velocity.GetY());
        //AZ_Printf("", "m_apply_velocity.GetZ() = %.10f", m_apply_velocity.GetZ());
        //AZ_Printf("", "m_sprint_increment_time = %.10f", m_sprint_increment_time);
        //AZ_Printf("", "m_sprint_value = %.10f", m_sprint_value);
        //AZ_Printf("", "m_sprint_accel_adjust = %.10f", m_sprint_accel_adjust);
        //AZ_Printf("", "m_sprint_velocity_adjust = %.10f", m_sprint_velocity_adjust);
        //AZ_Printf("", "m_sprint_held_duration = %.10f", m_sprint_held_duration);
        //AZ_Printf("", "m_sprint_decrement_pause = %.10f", m_sprint_decrement_pause);
        //AZ_Printf("", "m_sprint_prev_decrement_pause = %.10f", m_sprint_prev_decrement_pause);
        //AZ_Printf("", "m_sprint_cooldown = %.10f", m_sprint_cooldown);
        //static float prev_velocity = m_apply_velocity.GetLength();
        //AZ_Printf("", "dv/dt = %.10f", (m_apply_velocity.GetLength() - prev_velocity)/deltaTime);
        //prev_velocity = m_apply_velocity.GetLength();
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "X Position = %.10f", pos.GetX());
        //AZ_Printf("", "Y Position = %.10f", pos.GetY());
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("","");
    }

    void FirstPersonControllerComponent::CheckGrounded(const float& deltaTime)
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Used to determine when event notifications occur
        const bool prev_grounded = m_grounded;
        const bool prev_ground_close = m_ground_close;

        // Rotate the pose by 90 degrees on the Y axis since by default the capsule's height
        // is oriented along the X axis when we want it oriented along the Z axis
        AZ::Transform capsule_intersect_pose = AZ::Transform::CreateRotationY(AZ::Constants::HalfPi);
        // Move the capsule to the location of the character and apply the Z offset

        capsule_intersect_pose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsule_offset_translation));

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
            m_capsule_radius,
            m_capsule_height,
            capsule_intersect_pose,
            AZ::Vector3(0.f, 0.f, -1.f),
            0.f,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            AzPhysics::CollisionGroup::All,
            nullptr);

        request.m_reportMultipleHits = true;

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        AZStd::vector<AZ::Vector3> steep_normals;

        // Disregard intersections with the character's collider, its child entities,
        // and if the slope angle of the thing that's intersecting is greater than the max grounded angle
        auto self_child_slope_entity_check = [this, &steep_normals](AzPhysics::SceneQueryHit& hit)
            {
                if(hit.m_entityId == GetEntityId())
                    return true;

                // Obtain the child IDs if we don't already have them
                if(!m_obtained_child_ids)
                {
                    AZ::TransformBus::EventResult(m_children, GetEntityId(), &AZ::TransformBus::Events::GetChildren);
                    m_obtained_child_ids = true;
                }

                for(AZ::EntityId id: m_children)
                {
                    if(hit.m_entityId == id)
                        return true;
                }

                if(abs(hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ())) > m_max_grounded_angle_degrees)
                {
                    steep_normals.push_back(hit.m_normal);
                    //AZ_Printf("", "Steep Angle EntityId = %s", hit.m_entityId.ToString().c_str());
                    //AZ_Printf("", "Steep Angle = %.10f", hit.m_normal.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
                    return true;
                }

                return false;
            };

        AZStd::erase_if(hits.m_hits, self_child_slope_entity_check);
        m_grounded = hits ? true : false;

        // Check to see if the sum of the steep angles is less than or equal to m_max_grounded_angle_degrees
        if(!m_grounded && steep_normals.size() > 1)
        {
            AZ::Vector3 sum_normals = AZ::Vector3::CreateZero();
            for(AZ::Vector3 normal: steep_normals)
                sum_normals += normal;

            //AZ_Printf("", "Sum of Steep Angles = %.10f", sum_normals.AngleSafeDeg(AZ::Vector3::CreateAxisZ()));
            if(abs(sum_normals.AngleSafeDeg(AZ::Vector3::CreateAxisZ())) <= m_max_grounded_angle_degrees)
                m_grounded = true;
        }
        steep_normals.clear();

        if(m_script_set_ground_tick)
        {
            m_grounded = m_script_grounded;
            m_script_set_ground_tick = false;
        }

        if(m_grounded)
        {
            m_ground_close = true;
            m_air_time = 0.f;
        }
        // Check to see if the character is still close to the ground after pressing and holding the jump key
        // to allow them to jump higher based on the m_capsule_jump_hold_offset_translation distance
        else
        {
            m_air_time += deltaTime;

            capsule_intersect_pose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsule_jump_hold_offset_translation));

            request = AzPhysics::ShapeCastRequestHelpers::CreateCapsuleCastRequest(
                m_capsule_radius,
                m_capsule_height,
                capsule_intersect_pose,
                AZ::Vector3(0.f, 0.f, -1.f),
                0.f,
                AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
                AzPhysics::CollisionGroup::All,
                nullptr);

            request.m_reportMultipleHits = true;

            hits = sceneInterface->QueryScene(sceneHandle, &request);
            AZStd::erase_if(hits.m_hits, self_child_slope_entity_check);
            m_ground_close = hits ? true : false;

            if(m_script_set_ground_close_tick)
            {
                m_ground_close = m_script_ground_close;
                m_script_set_ground_close_tick = false;
            }
            //AZ_Printf("", "m_ground_close = %s", m_ground_close ? "true" : "false");
        }

        // Trigger an event notification if the player hits the ground, is about to hit the ground,
        // or just left the ground (via jumping or otherwise)
        if(!prev_grounded && m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundHit);
        else if(!prev_ground_close && m_ground_close)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnGroundSoonHit);
        else if(prev_grounded && !m_grounded)
            FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnUngrounded);
    }

    void FirstPersonControllerComponent::UpdateJumpTime()
    {
        // Calculate the amount of time that the jump key can be held based on m_capsule_jump_hold_offset
        // divided by the average of the initial jump velocity and the velocity at the edge of the capsule
        const float jump_velocity_capsule_edge_squared = m_jump_initial_velocity*m_jump_initial_velocity
                                                         + 2.f*m_gravity*m_jump_held_gravity_factor*m_capsule_jump_hold_offset;
        // If the initial velocity is large enough such that the apogee can be reached outside of the capsule
        // then compute how long the jump key is held while still inside the jump hold offset intersection capsule
        if(jump_velocity_capsule_edge_squared >= 0.f)
            m_jump_time = m_capsule_jump_hold_offset / ((m_jump_initial_velocity
                                                        + sqrt(jump_velocity_capsule_edge_squared)) / 2.f);
        // Otherwise the apogee will be reached inside m_capsule_jump_hold_offset
        // and the jump time needs to computed accordingly
        else
            m_jump_time = abs(m_jump_initial_velocity / (m_gravity*m_jump_held_gravity_factor));
    }

    void FirstPersonControllerComponent::UpdateVelocityZ(const float& deltaTime)
    {
        AZ::Vector3 current_velocity = AZ::Vector3::CreateZero();
        Physics::CharacterRequestBus::EventResult(current_velocity, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetVelocity);

        // Used for the Verlet integration averaging calculation
        m_z_velocity_prev_delta = m_z_velocity_current_delta;

        if(m_grounded && (m_jump_req_repress || (current_velocity.GetZ() <= 0.f && m_z_velocity <= 0.f)))
        {
            if(m_jump_value && !m_jump_held)
            {
                m_z_velocity_current_delta = m_jump_initial_velocity;
                m_jump_held = true;
                m_jump_req_repress = false;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnFirstJump);
            }
            else
            {
                m_z_velocity = 0.f;
                m_z_velocity_current_delta = 0.f;
                m_jump_counter = 0.f;

                if(m_jump_value == 0.f && m_jump_held)
                    m_jump_held = false;

                if(m_double_jump_enabled && m_second_jump)
                    m_second_jump = false;
            }
        }
        else if(m_jump_counter < m_jump_time && current_velocity.GetZ() > 0.f && m_jump_held && !m_jump_req_repress)
        {
            if(m_jump_value == 0.f)
            {
                m_jump_held = false;
                m_jump_counter = 0.f;
                m_z_velocity_current_delta = m_gravity * deltaTime;
            }
            else
            {
                m_jump_counter += deltaTime;
                m_z_velocity_current_delta = m_gravity * m_jump_held_gravity_factor * deltaTime;
            }
        }
        else
        {
            if(!m_jump_req_repress)
                m_jump_req_repress = true;

            if(m_jump_counter != 0.f)
                m_jump_counter = 0.f;

            if(current_velocity.GetZ() <= 0.f)
                m_z_velocity_current_delta = m_gravity * m_jump_falling_gravity_factor * deltaTime;
            else
                m_z_velocity_current_delta = m_gravity * deltaTime;

            if(!m_double_jump_enabled && !m_jump_held)
                m_jump_held = true;
            else if(m_double_jump_enabled && m_jump_value == 0.f && m_jump_held)
                m_jump_held = false;

            if(m_double_jump_enabled && !m_second_jump && !m_jump_held && m_jump_value != 0.f)
            {
                m_z_velocity = m_jump_initial_velocity;
                m_z_velocity_current_delta = 0.f;
                m_second_jump = true;
                m_jump_held = true;
                FirstPersonControllerNotificationBus::Broadcast(&FirstPersonControllerNotificationBus::Events::OnSecondJump);
            }
        }

        // Perform an average of the current and previous Z velocity delta
        // as described by Verlet integration, which should reduce accumulated error
        m_z_velocity += (m_z_velocity_current_delta + m_z_velocity_prev_delta) / 2.f;

        // Account for the case where the PhysX Character Gameplay component's gravity is used instead
        if(m_gravity == 0.f && m_grounded && current_velocity.GetZ() < 0.f)
            m_z_velocity = 0.f;

        // Debug print statements to observe the jump mechanic
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("", "current_velocity.GetZ() = %.10f", current_velocity.GetZ());
        //AZ_Printf("", "m_z_velocity_prev_delta = %.10f", m_z_velocity_prev_delta);
        //AZ_Printf("", "m_z_velocity_current_delta = %.10f", m_z_velocity_current_delta);
        //AZ_Printf("", "m_z_velocity = %.10f", m_z_velocity);
        //AZ_Printf("", "m_grounded = %s", m_grounded ? "true" : "false");
        //AZ_Printf("", "m_jump_counter = %.10f", m_jump_counter);
        //AZ_Printf("", "deltaTime = %.10f", deltaTime);
        //AZ_Printf("", "m_jump_time = %.10f", m_jump_time);
        //AZ_Printf("", "m_capsule_jump_hold_offset = %.10f", m_capsule_jump_hold_offset);
        //static float prev_z_velocity = m_z_velocity;
        //AZ_Printf("", "dvz/dt = %.10f", (m_z_velocity - prev_z_velocity)/deltaTime);
        //AZ_Printf("","");
        //prev_z_velocity = m_z_velocity;
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime)
    {
        UpdateRotation(deltaTime);

        CheckGrounded(deltaTime);

        if(m_grounded)
            CrouchManager(deltaTime);

        // So long as the character is grounded or depending on how the update X&Y velocity while jumping
        // boolean values are set, and based on the state of jumping/falling, update the X&Y velocity accordingly
        if(m_grounded || (m_update_xy_ascending && m_update_xy_descending && !m_update_xy_only_near_ground)
           || ((m_update_xy_ascending && m_z_velocity >= 0.f) && (!m_update_xy_only_near_ground || m_ground_close))
           || ((m_update_xy_descending && m_z_velocity <= 0.f) && (!m_update_xy_only_near_ground || m_ground_close)) )
            UpdateVelocityXY(deltaTime);

        UpdateVelocityZ(deltaTime);

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            (m_apply_velocity + AZ::Vector3::CreateAxisZ(m_z_velocity)));
    }

    // Event Notification methods for use in scripts
    void FirstPersonControllerComponent::OnGroundHit(){}
    void FirstPersonControllerComponent::OnGroundSoonHit(){}
    void FirstPersonControllerComponent::OnUngrounded(){}
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
        m_script_grounded = new_grounded;
        m_script_set_ground_tick = true;
    }
    bool FirstPersonControllerComponent::GetGroundClose() const
    {
        return m_ground_close;
    }
    void FirstPersonControllerComponent::SetGroundCloseForTick(const bool& new_ground_close)
    {
        m_script_ground_close = new_ground_close;
        m_script_set_ground_close_tick = true;
    }
    float FirstPersonControllerComponent::GetAirTime() const
    {
        return m_air_time;
    }
    float FirstPersonControllerComponent::GetJumpKeyValue() const
    {
        return m_jump_value;
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
        return m_jump_initial_velocity;
    }
    void FirstPersonControllerComponent::SetInitialJumpVelocity(const float& new_initial_jump_velocity)
    {
        m_jump_initial_velocity = new_initial_jump_velocity;
        UpdateJumpTime();
    }
    bool FirstPersonControllerComponent::GetDoubleJump() const
    {
        return m_double_jump_enabled;
    }
    void FirstPersonControllerComponent::SetDoubleJump(const bool& new_double_jump)
    {
        m_double_jump_enabled = new_double_jump;
    }
    float FirstPersonControllerComponent::GetMaxGroundedAngleDegrees() const
    {
        return m_max_grounded_angle_degrees;
    }
    void FirstPersonControllerComponent::SetMaxGroundedAngleDegrees(const float& new_max_grounded_angle_degrees)
    {
        m_max_grounded_angle_degrees = new_max_grounded_angle_degrees;
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
        return m_sprint_scale;
    }
    void FirstPersonControllerComponent::SetSprintScale(const float& new_sprint_scale)
    {
        m_sprint_scale = new_sprint_scale;
    }
    float FirstPersonControllerComponent::GetSprintHeldTime() const
    {
        return m_sprint_held_duration;
    }
    void FirstPersonControllerComponent::SetSprintHeldTime(const float& new_sprint_held_duration)
    {
        m_sprint_held_duration = new_sprint_held_duration;
    }
    float FirstPersonControllerComponent::GetSprintCooldown() const
    {
        return m_sprint_cooldown;
    }
    void FirstPersonControllerComponent::SetSprintCooldown(const float& new_sprint_cooldown)
    {
        m_sprint_cooldown = new_sprint_cooldown;
    }
    float FirstPersonControllerComponent::GetSprintPauseTime() const
    {
        return m_sprint_decrement_pause;
    }
    void FirstPersonControllerComponent::SetSprintPauseTime(const float& new_sprint_decrement_pause)
    {
        m_sprint_decrement_pause = new_sprint_decrement_pause;
    }
    bool FirstPersonControllerComponent::GetCrouching() const
    {
        return m_crouching;
    }
    void FirstPersonControllerComponent::SetCrouching(const bool& new_crouching)
    {
        m_crouching = new_crouching;
    }
    float FirstPersonControllerComponent::GetCrouchCameraDistance() const
    {
        return m_crouch_camera_distance;
    }
    void FirstPersonControllerComponent::SetCrouchCameraDistance(const float& new_crouch_camera_distance)
    {
        m_crouch_camera_distance = new_crouch_camera_distance;
    }
    float FirstPersonControllerComponent::GetCrouchCameraTime() const
    {
        return m_crouch_camera_time;
    }
    void FirstPersonControllerComponent::SetCrouchCameraTime(const float& new_crouch_camera_time)
    {
        m_crouch_camera_time = new_crouch_camera_time;
    }
    bool FirstPersonControllerComponent::GetCrouchEnableToggle() const
    {
        return m_crouch_enable_toggle;
    }
    void FirstPersonControllerComponent::SetCrouchEnableToggle(const bool& new_crouch_enable_toggle)
    {
        m_crouch_enable_toggle = new_crouch_enable_toggle;
    }
    float FirstPersonControllerComponent::GetCameraPitchSensitivity() const
    {
        return m_pitch_sensitivity;
    }
    void FirstPersonControllerComponent::SetCameraPitchSensitivity(const float& new_pitch_sensitivity)
    {
        m_pitch_sensitivity = new_pitch_sensitivity;
    }
    float FirstPersonControllerComponent::GetCameraYawSensitivity() const
    {
        return m_yaw_sensitivity;
    }
    void FirstPersonControllerComponent::SetCameraYawSensitivity(const float& new_yaw_sensitivity)
    {
        m_yaw_sensitivity = new_yaw_sensitivity;
    }
    float FirstPersonControllerComponent::GetCameraRotationDampFactor() const
    {
        return m_rotation_damp;
    }
    void FirstPersonControllerComponent::SetCameraRotationDampFactor(const float& new_rotation_damp)
    {
        m_rotation_damp = new_rotation_damp;
    }
    void FirstPersonControllerComponent::UpdateCameraPitch(const float& new_camera_pitch_angle)
    {
        m_camera_rotation_angles[0] = new_camera_pitch_angle - m_pitch_value * m_pitch_sensitivity;
        m_rotating_pitch_via_script_gamepad = true;
    }
    void FirstPersonControllerComponent::UpdateCameraYaw(const float& new_camera_yaw_angle)
    {
        m_camera_rotation_angles[2] = new_camera_yaw_angle - m_yaw_value * m_yaw_sensitivity;
        m_rotating_yaw_via_script_gamepad = true;
    }
    float FirstPersonControllerComponent::GetHeading() const
    {
        return m_current_heading;
    }
}
