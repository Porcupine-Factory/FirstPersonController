#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Components/CameraBus.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonControllerComponent::Reflect(AZ::ReflectContext* rc)
    {
        if(auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonControllerComponent, AZ::Component>()
              ->Field("Forward Key", &FirstPersonControllerComponent::m_str_forward)
              ->Field("Back Key", &FirstPersonControllerComponent::m_str_back)
              ->Field("Left Key", &FirstPersonControllerComponent::m_str_left)
              ->Field("Right Key", &FirstPersonControllerComponent::m_str_right)
              ->Field("Sprint Key", &FirstPersonControllerComponent::m_str_sprint)
              ->Field("Forward Scale", &FirstPersonControllerComponent::m_forward_scale)
              ->Field("Back Scale", &FirstPersonControllerComponent::m_back_scale)
              ->Field("Left Scale", &FirstPersonControllerComponent::m_left_scale)
              ->Field("Right Scale", &FirstPersonControllerComponent::m_right_scale)
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_str_yaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_str_pitch)
              ->Field("Camera Rotation Damp Factor", &FirstPersonControllerComponent::m_rotation_damp)
              ->Field("Top Walking Speed (m/s)", &FirstPersonControllerComponent::m_speed)
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yaw_sensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitch_sensitivity)
              ->Field("Walking Acceleration (m/s²)", &FirstPersonControllerComponent::m_accel)
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Breaking Factor", &FirstPersonControllerComponent::m_break)
              ->Field("Sprint Max Time (sec)", &FirstPersonControllerComponent::m_sprint_max_time)
              ->Field("Sprint Cooldown (sec)", &FirstPersonControllerComponent::m_sprint_cooldown_time)
              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonControllerComponent>("First Person Controller",
                    "[First person character controller]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person")
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
                        &FirstPersonControllerComponent::m_str_sprint,
                        "Sprint Key", "Key for sprinting")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_pitch,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_yaw,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_rotation_damp,
                        "Camera Rotation Damp Factor", "The damp factor applied to the camera rotation")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_speed,
                        "Top Walking Speed (m/s)", "Speed of the character")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yaw_sensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitch_sensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_accel,
                        "Walking Acceleration (m/s²)", "Acceleration")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_decel,
                        "Deceleration Factor", "Deceleration")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_break,
                        "Breaking Factor", "Breaking")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_max_time,
                        "Sprint Max Time (sec)", "Maximum Sprint Applied Time")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_cooldown_time,
                        "Sprint Cooldown (sec)", "Sprint Cooldown Time");
            }
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
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
    }

    void FirstPersonControllerComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void FirstPersonControllerComponent::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if(inputId == nullptr)
            return;

        if(*inputId == m_SprintEventId)
        {
            m_sprint_value = m_sprint_pressed_value = value;
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
            m_sprint_value = m_sprint_pressed_value = value;
        }
    }

    void FirstPersonControllerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime);
    }

    AZ::Entity* FirstPersonControllerComponent::GetActiveCamera()
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId,
            &Camera::CameraSystemRequestBus::Events::GetActiveCamera);

        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void FirstPersonControllerComponent::SlerpRotation(const float& deltaTime)
    {
        // Multiply by -1 since moving the mouse to the right produces a positive value
        // but a positive rotation about Z is counterclockwise
        const float angles[3] = {-1.f * m_pitch_value * m_pitch_sensitivity,
                                 0.f,
                                 -1.f * m_yaw_value * m_yaw_sensitivity};

        const AZ::Quaternion target_look_direction = AZ::Quaternion::CreateFromEulerAnglesRadians(
            AZ::Vector3::CreateFromFloat3(angles));

        m_new_look_direction = m_new_look_direction.Slerp(target_look_direction, m_rotation_damp*deltaTime);
    }

    void FirstPersonControllerComponent::UpdateRotation(const float& deltaTime)
    {
        AZ::TransformInterface* t = GetEntity()->GetTransform();

        SlerpRotation(deltaTime);
        const AZ::Vector3 new_look_direction = m_new_look_direction.GetEulerRadians();

        t->RotateAroundLocalZ(new_look_direction.GetZ());

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        const float rotate_pitch = -1.f * m_pitch_value * m_pitch_sensitivity;
        const float current_pitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(current_pitch) <= Pi/2.f ||
           current_pitch >= Pi/2.f && rotate_pitch < 0.f ||
           current_pitch <= -Pi/2.f && rotate_pitch > 0.f)
        {
            t->RotateAroundLocalX(new_look_direction.GetX());
        }

        m_current_heading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();
    }

    AZ::Vector3 FirstPersonControllerComponent::LerpVelocity(const AZ::Vector3& target_velocity, const float& deltaTime)
    {
        float total_lerp_time = m_last_applied_velocity.GetDistance(target_velocity)/m_accel;

        // Apply the sprint factor to the acceleration (dt) based on the sprint having been (recently) pressed
        const float last_lerp_time = m_lerp_time;
        m_lerp_time += m_sprint_time > 0.f ? deltaTime * (1.f + (m_sprint_pressed_value-1.f) * m_sprint_accel_adjust) : deltaTime;

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
            if(target_velocity.GetLength() != 0.f && abs(apply_velocity_world.AngleDeg(target_velocity)) > 90.f)
                deceleration_factor *= m_break;

            // Use the deceleration factor to get the lerp time closer to the total lerp time at a faster rate
            m_lerp_time = last_lerp_time + (m_lerp_time - last_lerp_time) * deceleration_factor;

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
            m_sprint_time = 0.f;
        }

        const float total_sprint_time = ((m_sprint_value-1.f)*m_speed)/m_accel;

        // If the sprint key is pressed then increment the sprint counter
        if(m_sprint_value != 1.f && m_sprint_held_duration < m_sprint_max_time && m_sprint_cooldown == 0.f)
        {
            // Sprint adjustment factor based on the angle of the target velocity
            // with respect to their frame of reference
            m_sprint_velocity_adjust = 1.f - target_velocity.Angle(AZ::Vector3::CreateAxisY())/(AZ::Constants::Pi/2.f);
            m_sprint_accel_adjust = m_sprint_velocity_adjust;

            m_sprint_time += deltaTime;
            m_sprint_held_duration += deltaTime;

            if(m_sprint_time > total_sprint_time)
                m_sprint_time = total_sprint_time;
        }
        // Otherwise if the sprint key isn't pressed then decrement the sprint counter
        else if(m_sprint_value == 1.f || m_sprint_held_duration >= m_sprint_max_time || m_sprint_cooldown != 0.f)
        {
            // Set the sprint velocity adjust to 0
            m_sprint_velocity_adjust = 0.f;

            m_sprint_time -= deltaTime;
            if(m_sprint_time < 0.f)
                m_sprint_time = 0.f;

            if(m_sprint_held_duration >= m_sprint_max_time && m_sprint_cooldown == 0.f)
            {
                m_sprint_held_duration = 0.f;
                m_sprint_cooldown = m_sprint_cooldown_time - deltaTime;
            }
            else
            {
                m_sprint_cooldown -= deltaTime;
                if(m_sprint_cooldown < 0.f)
                    m_sprint_cooldown = 0.f;
            }
        }
    }

    void FirstPersonControllerComponent::UpdateVelocity(const float& deltaTime)
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
        target_velocity *= m_speed * (1.f + (m_sprint_value-1.f) * m_sprint_velocity_adjust);

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

        // Debug print statements to observe the velocity and acceleration
        //AZ_Printf("", "m_current_heading = %.10f", m_current_heading);
        //AZ_Printf("", "atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()) = %.10f", atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()));
        //AZ_Printf("", "m_apply_velocity.GetLength() = %.10f", m_apply_velocity.GetLength());
        //AZ_Printf("", "m_apply_velocity.GetX() = %.10f", m_apply_velocity.GetX());
        //AZ_Printf("", "m_apply_velocity.GetY() = %.10f", m_apply_velocity.GetY());
        //AZ_Printf("", "m_sprint_time = %.10f", m_sprint_time);
        //AZ_Printf("", "m_sprint_value = %.10f", m_sprint_value);
        //AZ_Printf("", "m_sprint_accel_adjust = %.10f", m_sprint_accel_adjust);
        //AZ_Printf("", "m_sprint_velocity_adjust = %.10f", m_sprint_velocity_adjust);
        //AZ_Printf("", "m_sprint_held_duration = %.10f", m_sprint_held_duration);
        //AZ_Printf("", "m_sprint_cooldown = %.10f", m_sprint_cooldown);
        //static float prev_velocity = m_apply_velocity.GetLength();
        //AZ_Printf("", "dv/dt = %.10f", (m_apply_velocity.GetLength() - prev_velocity));
        //prev_velocity = m_apply_velocity.GetLength();

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick, m_apply_velocity);
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime)
    {
        UpdateRotation(deltaTime);
        UpdateVelocity(deltaTime);
    }
}
