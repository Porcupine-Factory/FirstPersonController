#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/PhysicsScene.h>
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
              ->Field("Jump Key", &FirstPersonControllerComponent::m_str_jump)
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
              ->Field("XY Acceleration Jump Factor (m/s²)", &FirstPersonControllerComponent::m_jump_accel_factor)
              ->Field("Gravity (m/s²)", &FirstPersonControllerComponent::m_gravity)
              ->Field("Deceleration Factor", &FirstPersonControllerComponent::m_decel)
              ->Field("Breaking Factor", &FirstPersonControllerComponent::m_break)
              ->Field("Sprint Max Time (sec)", &FirstPersonControllerComponent::m_sprint_max_time)
              ->Field("Sprint Cooldown (sec)", &FirstPersonControllerComponent::m_sprint_cooldown_time)
              ->Field("Capsule Overlap Height (m)", &FirstPersonControllerComponent::m_capsule_height)
              ->Field("Capsule Overlap Radius (m)", &FirstPersonControllerComponent::m_capsule_radius)
              ->Field("Capsule Overlap Offset (m)", &FirstPersonControllerComponent::m_capsule_offset)
              ->Field("Capsule Jump Hold Offset (m)", &FirstPersonControllerComponent::m_capsule_jump_hold_offset)
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
                        &FirstPersonControllerComponent::m_str_jump,
                        "Jump Key", "Key for jumping")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_yaw,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_pitch,
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
                        &FirstPersonControllerComponent::m_jump_accel_factor,
                        "XY Acceleration Jump Factor (m/s²)", "X & Y acceleration factor while jumping but still close to the ground")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_gravity,
                        "Gravity (m/s²)", "Z Acceleration due to gravity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_decel,
                        "Deceleration Factor", "Deceleration multiplier")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_break,
                        "Breaking Factor", "Breaking multiplier")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_max_time,
                        "Sprint Max Time (sec)", "Maximum Sprint Applied Time")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_sprint_cooldown_time,
                        "Sprint Cooldown (sec)", "Sprint Cooldown Time")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_height,
                        "Capsule Overlap Height (m)", "The ground detect capsule overlap height in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_radius,
                        "Capsule Overlap Radius (m)", "The ground detect capsule overlap radius in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_offset,
                        "Capsule Overlap Offset (m)", "The capsule overlap's ground detect offset in meters")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_capsule_jump_hold_offset,
                        "Capsule Jump Hold Offset (m)", "The capsule's jump hold offset in meters");
            }
        }

        if(auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<FirstPersonControllerNotificationBus>("FirstPersonNotificationBus")
                ->Handler<FirstPersonControllerNotificationHandler>();

            bc->EBus<FirstPersonControllerComponentRequestBus>("FirstPersonControllerComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "FirstPerson")
                ->Event("GetGrounded", &FirstPersonControllerComponentRequests::GetGrounded)
                ->Event("GetGroundClose", &FirstPersonControllerComponentRequests::GetGroundClose);

            bc->Class<FirstPersonControllerComponent>()->RequestBus("FirstPersonControllerComponentRequestBus");
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
        m_capsule_offset = m_capsule_height/2.f - m_capsule_offset;
        m_capsule_jump_hold_offset = m_capsule_height/2.f - m_capsule_jump_hold_offset;

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
        FirstPersonControllerComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void FirstPersonControllerComponent::Deactivate()
    {
        InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        FirstPersonControllerComponentRequestBus::Handler::BusDisconnect();
    }

    void FirstPersonControllerComponent::GetRequredServices(AZ::ComponentDescriptor::DependencyArrayType& req)
    {
        // This component requires the Transform Service
        req.push_back(AZ_CRC_CE("TransformService"));
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

        if(m_rotation_damp*deltaTime <= 1.f)
            m_new_look_direction = m_new_look_direction.Slerp(target_look_direction, m_rotation_damp*deltaTime);
        else
            m_new_look_direction = target_look_direction;
    }

    void FirstPersonControllerComponent::UpdateRotation(const float& deltaTime)
    {
        AZ::TransformInterface* t = GetEntity()->GetTransform();

        SlerpRotation(deltaTime);
        const AZ::Vector3 new_look_direction = m_new_look_direction.GetEulerRadians();

        t->RotateAroundLocalZ(new_look_direction.GetZ());

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        float current_pitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(current_pitch) <= HalfPi ||
           current_pitch >= HalfPi && new_look_direction.GetX() < 0.f ||
           current_pitch <= -HalfPi && new_look_direction.GetX() > 0.f)
        {
            t->RotateAroundLocalX(new_look_direction.GetX());
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

        float lerp_deltaTime = m_sprint_time > 0.f ? deltaTime * (1.f + (m_sprint_pressed_value-1.f) * m_sprint_accel_adjust) : deltaTime;
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
            m_sprint_velocity_adjust = 1.f - target_velocity.Angle(AZ::Vector3::CreateAxisY())/(AZ::Constants::HalfPi);
            m_sprint_accel_adjust = m_sprint_velocity_adjust;

            m_sprint_time += deltaTime;
            m_sprint_held_duration += deltaTime * m_sprint_velocity_adjust;

            m_sprint_decrementing = false;

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
                m_sprint_cooldown = m_sprint_cooldown_time;
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
            else if(m_sprint_cooldown_time < m_sprint_max_time)
            {
                m_sprint_decrement_pause -= deltaTime;
                // The use of m_sprint_decrement_pause here is somewhat unnecessary since when
                // m_sprint_cooldown_time < m_sprint_max_time it is advantageous to always hold down the sprint
                // key if the goal is to maximize the average velocity over time. Using m_sprint_decrement_pause
                // here is simply to deter spamming the key in an attempt to maintain it from elapsing.
                if(m_sprint_held_duration > 0.f && !m_sprint_decrementing)
                {
                    // Making m_sprint_decrement_pause = m_sprint_cooldown_time * 0.1 is arbitrary,
                    // this can be set to any other desired number if you have
                    // m_sprint_cooldown_time < m_sprint_max_time.
                    // It is not exposed in the editor since it would only be desirable to make it a constant
                    // in the condition where m_sprint_cooldown_time < m_sprint_max_time
                    m_sprint_decrement_pause = m_sprint_cooldown_time * 0.1f;
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
        }
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

        // Debug print statements to observe the velocity, acceleration, and position
        //AZ_Printf("", "m_current_heading = %.10f", m_current_heading);
        //AZ_Printf("", "atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()) = %.10f", atan(m_apply_velocity.GetY()/m_apply_velocity.GetX()));
        //AZ_Printf("", "m_apply_velocity.GetLength() = %.10f", m_apply_velocity.GetLength());
        //AZ_Printf("", "m_apply_velocity.GetX() = %.10f", m_apply_velocity.GetX());
        //AZ_Printf("", "m_apply_velocity.GetY() = %.10f", m_apply_velocity.GetY());
        //AZ_Printf("", "m_apply_velocity.GetZ() = %.10f", m_apply_velocity.GetZ());
        //AZ_Printf("", "m_sprint_time = %.10f", m_sprint_time);
        //AZ_Printf("", "m_sprint_value = %.10f", m_sprint_value);
        //AZ_Printf("", "m_sprint_accel_adjust = %.10f", m_sprint_accel_adjust);
        //AZ_Printf("", "m_sprint_velocity_adjust = %.10f", m_sprint_velocity_adjust);
        //AZ_Printf("", "m_sprint_held_duration = %.10f", m_sprint_held_duration);
        //AZ_Printf("", "m_sprint_decrement_pause = %.10f", m_sprint_decrement_pause);
        //AZ_Printf("", "m_sprint_cooldown = %.10f", m_sprint_cooldown);
        //static float prev_velocity = m_apply_velocity.GetLength();
        //AZ_Printf("", "dv/dt = %.10f", (m_apply_velocity.GetLength() - prev_velocity));
        //prev_velocity = m_apply_velocity.GetLength();
        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "X Position = %.10f", pos.GetX());
        //AZ_Printf("", "Y Position = %.10f", pos.GetY());
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
    }

    void FirstPersonControllerComponent::OnGroundHit(){}
    void FirstPersonControllerComponent::OnGroundSoonHit(){}
    void FirstPersonControllerComponent::OnUngrounded(){}
    bool FirstPersonControllerComponent::GetGrounded() const
    {
        return m_grounded;
    }
    bool FirstPersonControllerComponent::GetGroundClose() const
    {
        return m_ground_close;
    }

    void FirstPersonControllerComponent::CheckGrounded()
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();

        // Used to determine when event notifications occur
        const bool prev_grounded = m_grounded;
        const bool prev_ground_close = m_ground_close;

        // Rotate the pose by 90 degrees on the Y axis since by default the capsule's height
        // is oriented along the X axis when we want it oriented along the Z axis
        AZ::Transform capsule_intersect_pose = AZ::Transform::CreateRotationY(AZ::Constants::HalfPi);
        // Move the capsule to the location of the character and apply the Z offset
        capsule_intersect_pose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsule_offset));

        AzPhysics::OverlapRequest request = AzPhysics::OverlapRequestHelpers::CreateCapsuleOverlapRequest(
            m_capsule_height,
            m_capsule_radius,
            capsule_intersect_pose,
            nullptr);

        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        // Disregard intersections with the character's collider and its child entities
        auto self_child_entity_check = [this](const AzPhysics::SceneQueryHit& hit)
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
                    if(hit.m_entityId == id)
                        return true;

                return false;
            };

        AZStd::erase_if(hits.m_hits, self_child_entity_check);
        m_grounded = hits ? true : false;

        if(m_grounded)
            m_ground_close = true;
        // Check to see if the character is still close to the ground after pressing and holding the jump key
        // to allow them to jump higher based on the m_capsule_jump_hold_offset distance
        else
        {
            capsule_intersect_pose.SetTranslation(GetEntity()->GetTransform()->GetWorldTM().GetTranslation() + AZ::Vector3::CreateAxisZ(m_capsule_jump_hold_offset));

            request = AzPhysics::OverlapRequestHelpers::CreateCapsuleOverlapRequest(
                m_capsule_height,
                m_capsule_radius,
                capsule_intersect_pose,
                nullptr);

            hits = sceneInterface->QueryScene(sceneHandle, &request);
            AZStd::erase_if(hits.m_hits, self_child_entity_check);
            m_ground_close = hits ? true : false;
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

    void FirstPersonControllerComponent::UpdateVelocityZ(const float& deltaTime)
    {
        AZ::Vector3 current_velocity = AZ::Vector3::CreateZero();
        Physics::CharacterRequestBus::EventResult(current_velocity, GetEntityId(),
            &Physics::CharacterRequestBus::Events::GetVelocity);

        if(m_grounded && (m_jump_req_repress || current_velocity.GetZ() <= 0.f))
        {
            if(m_jump_value && !m_jump_held)
            {
                m_z_velocity = m_jump_value;
                m_jump_held = true;
                m_jump_req_repress = false;
            }
            else
            {
                m_z_velocity = 0.f;
                if(m_jump_value == 0.f && m_jump_held)
                    m_jump_held = false;
            }
        }
        else if(m_ground_close && current_velocity.GetZ() > 0.f && m_jump_held && !m_jump_req_repress)
        {
            if(m_jump_value == 0.f)
                m_jump_held = false;
            else
                m_z_velocity = m_jump_value;
        }
        else
        {
            if(!m_jump_req_repress)
                m_jump_req_repress = true;

            m_z_velocity += m_gravity * deltaTime;

            // Account for the case where the PhysX Character Gameplay component's gravity is used instead
            if(m_gravity == 0.f && m_grounded && current_velocity.GetZ() < 0.f)
                m_z_velocity = 0.f;
        }

        //AZ::Vector3 pos = GetEntity()->GetTransform()->GetWorldTM().GetTranslation();
        //AZ_Printf("", "Z Position = %.10f", pos.GetZ());
        //AZ_Printf("", "current_velocity.GetZ() = %.10f", current_velocity.GetZ());
        //AZ_Printf("", "m_z_velocity = %.10f", m_z_velocity);
        //AZ_Printf("", "m_grounded = %s", m_grounded ? "true" : "false");
    }

    void FirstPersonControllerComponent::ProcessInput(const float& deltaTime)
    {
        UpdateRotation(deltaTime);

        CheckGrounded();

        if(m_ground_close)
            UpdateVelocityXY(deltaTime);

        UpdateVelocityZ(deltaTime);

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            (m_apply_velocity + AZ::Vector3::CreateAxisZ(m_z_velocity)));
    }
}
