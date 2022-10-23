#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/string/string.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    class FirstPersonInput
    {
    public:
        float m_forwardAxis = 0;
        float m_strafeAxis = 0;
        float m_viewYaw = 0;
        float m_viewPitch = 0;
    };

    class FirstPersonControllerComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT(FirstPersonControllerComponent, "{0a47c7c2-0f94-48dd-8e3f-fd55c30475b9}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

    private:
        StartingPointInput::InputEventNotificationId m_MoveForwardEventId;
        AZStd::string m_str_Forward;
        StartingPointInput::InputEventNotificationId m_MoveBackEventId;
        AZStd::string m_str_Back;
        StartingPointInput::InputEventNotificationId m_MoveLeftEventId;
        AZStd::string m_str_Left;
        StartingPointInput::InputEventNotificationId m_MoveRightEventId;
        AZStd::string m_str_Right;
        StartingPointInput::InputEventNotificationId m_RotateYawEventId;
        AZStd::string m_str_Yaw;
        StartingPointInput::InputEventNotificationId m_RotatePitchEventId;
        AZStd::string m_str_Pitch;
    };
}
