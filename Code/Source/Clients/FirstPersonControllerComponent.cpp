#include <AzCore/Component/TickBus.h>
#include <Clients/FirstPersonControllerComponent.h>
#include <AzCore/Serialization/EditContext.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void FirstPersonControllerComponent::Reflect(AZ::ReflectContext* rc)
    {
        if(auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<FirstPersonControllerComponent, AZ::Component>()
              ->Field("Forward Key", &FirstPersonControllerComponent::m_str_Forward)
              ->Field("Back Key", &FirstPersonControllerComponent::m_str_Back)
              ->Field("Left Key", &FirstPersonControllerComponent::m_str_Left)
              ->Field("Right Key", &FirstPersonControllerComponent::m_str_Right)
              ->Field("Camera Yaw Rotate Input", &FirstPersonControllerComponent::m_str_Yaw)
              ->Field("Camera Pitch Rotate Input", &FirstPersonControllerComponent::m_str_Pitch)
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
                        &FirstPersonControllerComponent::m_str_Forward,
                        "Forward Key", "Key for moving forward")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_Back,
                        "Back Key", "Key for moving back")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_Left,
                        "Left Key", "Key for moving left")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_Right,
                        "Right Key", "Key for moving right")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_Pitch,
                        "Camera Yaw Rotate Input", "Camera yaw rotation control")
                    ->DataElement(nullptr,
                        &FirstPersonControllerComponent::m_str_Yaw,
                        "Camera Pitch Rotate Input", "Camera pitch rotation control");
            }
        }

    }

    void FirstPersonControllerComponent::Activate()
    {
        m_MoveForwardEventId = StartingPointInput::InputEventNotificationId(m_str_Forward.c_str());
        m_MoveBackEventId = StartingPointInput::InputEventNotificationId(m_str_Back.c_str());
        m_MoveLeftEventId = StartingPointInput::InputEventNotificationId(m_str_Left.c_str());
        m_MoveRightEventId = StartingPointInput::InputEventNotificationId(m_str_Right.c_str());
        m_RotateYawEventId = StartingPointInput::InputEventNotificationId(m_str_Yaw.c_str());
        m_RotatePitchEventId = StartingPointInput::InputEventNotificationId(m_str_Pitch.c_str());
        InputEventNotificationBus::MultiHandler::BusConnect(m_MoveForwardEventId);
        InputEventNotificationBus::MultiHandler::BusConnect(m_MoveBackEventId);
        InputEventNotificationBus::MultiHandler::BusConnect(m_MoveLeftEventId);
        InputEventNotificationBus::MultiHandler::BusConnect(m_MoveRightEventId);
        InputEventNotificationBus::MultiHandler::BusConnect(m_RotateYawEventId);
        InputEventNotificationBus::MultiHandler::BusConnect(m_RotatePitchEventId);
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
        {
            return;
        }

        if(*inputId == m_MoveForwardEventId)
        {
            AZ_Printf("", "Forward");
        }
        else if(*inputId == m_MoveBackEventId)
        {
            AZ_Printf("", "Back");
        }
        else if(*inputId == m_MoveLeftEventId)
        {
            AZ_Printf("", "Left");
        }
        else if(*inputId == m_MoveRightEventId)
        {
            AZ_Printf("", "Right");
        }
        else if(*inputId == m_RotateYawEventId)
        {
            AZ_Printf("", "Yaw");
        }
        else if(*inputId == m_RotatePitchEventId)
        {
            AZ_Printf("", "Pitch");
        }
    }

    void FirstPersonControllerComponent::OnTick(float, AZ::ScriptTimePoint)
    {
        ;
    }
}
