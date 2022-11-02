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
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_str_yaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_str_pitch)
              ->Field("Top Walking Speed (m/s)", &FirstPersonControllerComponent::m_speed)
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yaw_sensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitch_sensitivity)
              ->Field("Walking Acceleration (m/s²)", &FirstPersonControllerComponent::m_accel)
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
                        &FirstPersonControllerComponent::m_str_sprint,
                        "Sprint Key", "Key for sprinting")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_pitch,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_yaw,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control")
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
                        "Walking Acceleration (m/s²)", "Acceleration/deceleration");
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

        for(auto& it_event : m_control_map)
        {
            if(*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Pressed", it_event.first->ToString().c_str());
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
                // AZ_Printf("Released", it_event->first->ToString().c_str());
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

    void FirstPersonControllerComponent::UpdateRotation()
    {
        AZ::TransformInterface* t = GetEntity()->GetTransform();

        // Multiply by -1 since moving the mouse to the right produces a positive value
        // but a positive rotation about Z is counterclockwise
        t->RotateAroundLocalZ(-1 * m_yaw_value * m_yaw_sensitivity);

        m_activeCameraEntity = GetActiveCamera();
        t = m_activeCameraEntity->GetTransform();

        const float rotate_pitch = -1 * m_pitch_value * m_pitch_sensitivity;
        const float current_pitch = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(current_pitch) <= Pi/2 ||
           current_pitch >= Pi/2 && rotate_pitch < 0 ||
           current_pitch <= -Pi/2 && rotate_pitch > 0)
        {
            t->RotateAroundLocalX(-1 * m_pitch_value * m_pitch_sensitivity);
        }
    }

    void FirstPersonControllerComponent::LerpMovement(float deltaTime)
    {
        // The sprint value should never be 0
        if(m_sprint_value == 0)
            m_sprint_value = 1.f;

        // Lerp movements
        for(int dir = 0; dir < sizeof(m_directions_lerp) / sizeof(m_directions_lerp[0]); ++dir)
        {
            using namespace LerpAccess;

            // Obtain the total ramp time based on the acceleration and top walk speed
            float total_ramp_time = abs(*m_directions_lerp[dir][value]*m_speed)/m_accel;

            if(abs(*m_directions_lerp[dir][value]) > abs(*m_directions_lerp[dir][current_lerp_value]) &&
               *m_directions_lerp[dir][ramp_time] < total_ramp_time)
            {
                *m_directions_lerp[dir][ramp_time] += deltaTime;
            }
            else if(abs(*m_directions_lerp[dir][value]) < abs(*m_directions_lerp[dir][current_lerp_value]) &&
                    *m_directions_lerp[dir][ramp_time] > 0.f)
            {
                *m_directions_lerp[dir][ramp_time] -= deltaTime;
            }

            if(abs(*m_directions_lerp[dir][current_lerp_value]) < abs(*m_directions_lerp[dir][value]))
            {
                if(!*m_pressed[dir])
                {
                    *m_pressed[dir] = true;
                    *m_directions_lerp[dir][last_lerp_value] = *m_directions_lerp[dir][current_lerp_value];
                    *m_directions_lerp[dir][ramp_pressed_released_time] = *m_directions_lerp[dir][ramp_time];
                }
                //AZ_Printf("", "SPEEDING UP");

                *m_directions_lerp[dir][current_lerp_value] =
                    AZ::Lerp(*m_directions_lerp[dir][last_lerp_value],
                             *m_directions_lerp[dir][value],
                             ((*m_directions_lerp[dir][ramp_time] - *m_directions_lerp[dir][ramp_pressed_released_time])
                              / (total_ramp_time - *m_directions_lerp[dir][ramp_pressed_released_time])));

                if(abs(*m_directions_lerp[dir][current_lerp_value]) > abs(*m_directions_lerp[dir][value]))
                    *m_directions_lerp[dir][current_lerp_value] = *m_directions_lerp[dir][value];
            }
            else if(abs(*m_directions_lerp[dir][current_lerp_value]) > abs(*m_directions_lerp[dir][value]))
            {
                if(*m_pressed[dir])
                {
                    *m_pressed[dir] = false;
                    *m_directions_lerp[dir][last_lerp_value] = *m_directions_lerp[dir][current_lerp_value];
                    *m_directions_lerp[dir][ramp_pressed_released_time] = *m_directions_lerp[dir][ramp_time];
                }
                //AZ_Printf("", "SLOWING DOWN");

                *m_directions_lerp[dir][current_lerp_value] =
                    AZ::Lerp(*m_directions_lerp[dir][value],
                             *m_directions_lerp[dir][last_lerp_value],
                             (*m_directions_lerp[dir][ramp_time] / *m_directions_lerp[dir][ramp_pressed_released_time]));

                if((*m_directions_lerp[dir][last_lerp_value] > *m_directions_lerp[dir][value] &&
                    *m_directions_lerp[dir][current_lerp_value] < *m_directions_lerp[dir][value]) ||
                   (*m_directions_lerp[dir][last_lerp_value] < *m_directions_lerp[dir][value] &&
                    *m_directions_lerp[dir][current_lerp_value] > *m_directions_lerp[dir][value]))
                {
                    *m_directions_lerp[dir][current_lerp_value] = *m_directions_lerp[dir][value];
                }
            }
        }
    }

    void FirstPersonControllerComponent::UpdateVelocity(float deltaTime)
    {
        const float currentHeading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();

        LerpMovement(deltaTime);

        const float forwardBack = m_current_forward_lerp_value + m_current_back_lerp_value;
        const float leftRight = m_current_left_lerp_value + m_current_right_lerp_value;

        // Decelerate the sprint modifier if there is no movement or if moving backwards
        if(forwardBack <= 0.f && !leftRight)
        {
            m_sprint_value = m_current_sprint_lerp_value = m_last_sprint_lerp_value = 1.f;
            m_sprint_ramp_time = m_sprint_release_ramp_time = 0.f;
        }

        AZ::Vector3 move = AZ::Vector3::CreateZero();

        if(forwardBack && leftRight)
            // Scale the rectangular movement to fit within the unit circle
            move = AZ::Vector3(leftRight * static_cast<float>(AZStd::sqrt(1 - 0.5*forwardBack*forwardBack)),
                               forwardBack * static_cast<float>(AZStd::sqrt(1 - 0.5*leftRight*leftRight)), 0.f);
        else
            move = AZ::Vector3(leftRight, forwardBack, 0.f);

        if(m_current_sprint_lerp_value != 1 && !m_back_value)
            m_velocity = AZ::Quaternion::CreateRotationZ(currentHeading).TransformVector(move) * m_speed * m_current_sprint_lerp_value;
        else
            m_velocity = AZ::Quaternion::CreateRotationZ(currentHeading).TransformVector(move) * m_speed;

        //AZ_Printf("", "m_velocity.GetLength() = %.10f", m_velocity.GetLength());

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick, m_velocity);
    }

    void FirstPersonControllerComponent::ProcessInput(float deltaTime)
    {
        UpdateRotation();
        UpdateVelocity(deltaTime);
    }
}
