#include <Clients/FirstPersonControllerComponent.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>

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
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_str_yaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_str_pitch)
              ->Field("Speed", &FirstPersonControllerComponent::m_speed)
              ->Field("Yaw Sensitivity", &FirstPersonControllerComponent::m_yaw_sensitivity)
              ->Field("Pitch Sensitivity", &FirstPersonControllerComponent::m_pitch_sensitivity)
              ->Version(1);

            if(AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<FirstPersonControllerComponent>("First Person Controller",
                    "[First person character cntroller]")
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
                        &FirstPersonControllerComponent::m_str_pitch,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_yaw,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_speed,
                        "Speed", "Speed of the character")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_yaw_sensitivity,
                        "Yaw Sensitivity", "Camera left/right rotation sensitivity")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_pitch_sensitivity,
                        "Pitch Sensitivity", "Camera up/down rotation sensitivity");
            }
        }
    }

    void FirstPersonControllerComponent::Activate()
    {
        if (m_control_map.size() != m_input_names.size())
        {
            AZ_Printf("FirstPersonControllerComponent",
                      "Number of input IDs not equal to number of input names!");
        }
        else
        {
            AZStd::map<StartingPointInput::InputEventNotificationId*, float*>::iterator it_event =
                m_control_map.begin();
            AZStd::vector<AZStd::string*>::iterator it_name =
                m_input_names.begin();
            for(; it_event != m_control_map.end(); ++it_event, ++it_name)
            {
                *(it_event->first) = StartingPointInput::InputEventNotificationId((*it_name)->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event->first));
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
        if (inputId == nullptr)
            return;

        AZStd::map<StartingPointInput::InputEventNotificationId*, float*>::iterator it_event;
        for(it_event = m_control_map.begin(); it_event != m_control_map.end(); ++it_event)
        {
            if (*inputId == *(it_event->first))
            {
                *(it_event->second) = value;
                // print the local user ID and the action name CRC
                //AZ_Printf("Pressed", it_event->first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        AZStd::map<StartingPointInput::InputEventNotificationId*, float*>::iterator it_event;
        for(it_event = m_control_map.begin(); it_event != m_control_map.end(); ++it_event)
        {
            if (*inputId == *(it_event->first))
            {
                *(it_event->second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Released", it_event->first->ToString().c_str());
            }
        }
    }

    void FirstPersonControllerComponent::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
        {
            return;
        }

        if (*inputId == m_RotateYawEventId)
        {
            m_yaw_value = value;
        }
        else if (*inputId == m_RotatePitchEventId)
        {
            m_pitch_value = value;
        }
    }

    void FirstPersonControllerComponent::OnTick(float, AZ::ScriptTimePoint)
    {
        ProcessInput();
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

        const float rotate_yaw = -1 * m_pitch_value * m_pitch_sensitivity;
        const float current_yaw = t->GetLocalRotation().GetX();

        using namespace AZ::Constants;
        if(abs(current_yaw) <= Pi/2 ||
           current_yaw >= Pi/2 && rotate_yaw < 0 ||
           current_yaw <= -Pi/2 && rotate_yaw > 0)
        {
            t->RotateAroundLocalX(-1 * m_pitch_value * m_pitch_sensitivity);
        }
    }

    void FirstPersonControllerComponent::UpdateVelocity()
    {
        const float currentHeading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();

        const float forwardBack = m_forward_value + m_back_value;
        const float leftRight = m_left_value + m_right_value;

        AZ::Vector3 move = AZ::Vector3::CreateZero();

        if(forwardBack && leftRight)
            move = AZ::Vector3(leftRight/sqrt(2), forwardBack/sqrt(2), 0.f);
        else
            move = AZ::Vector3(leftRight, forwardBack, 0.f);

        m_velocity = AZ::Quaternion::CreateRotationZ(currentHeading).TransformVector(move) * m_speed;

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick, m_velocity);
    }

    void FirstPersonControllerComponent::ProcessInput()
    {
        UpdateRotation();
        UpdateVelocity();
    }
}
