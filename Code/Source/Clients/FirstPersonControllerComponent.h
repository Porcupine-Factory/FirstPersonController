#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/map.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    static constexpr float Sqrt2 = 1.4142135623730950488016887242097;

    namespace LerpAccess
    {
        enum values {value, current_lerp_value, last_lerp_value, ramp_time, ramp_pressed_released_time};
    }

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
        void OnReleased(float value) override;
        void OnHeld(float value) override;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

    private:
        AZ::Entity* m_activeCameraEntity = nullptr;
        AZ::Entity* GetActiveCamera();

        void ProcessInput(float deltaTime);

        void UpdateVelocity(float deltaTime);
        void LerpMovement(float deltaTime);
        AZ::Vector3 m_velocity = AZ::Vector3::CreateZero();
        float m_speed = 10.f;

        void UpdateRotation();
        // These default values work well
        // assuming the event value multiplier is 1.0
        float m_yaw_sensitivity = 0.005f;
        float m_pitch_sensitivity = 0.005f;

        // Acceleration lerp movement
        float m_accel = 1.f;

        float m_current_forward_lerp_value = 0.f;
        float m_last_forward_lerp_value = 0.f;
        float m_forward_ramp_time = 0.f;
        float m_forward_release_ramp_time = 0.f;

        float m_current_back_lerp_value = 0.f;
        float m_last_back_lerp_value = 0.f;
        float m_back_ramp_time = 0.f;
        float m_back_release_ramp_time = 0.f;

        float m_current_left_lerp_value = 0.f;
        float m_last_left_lerp_value = 0.f;
        float m_left_ramp_time = 0.f;
        float m_left_release_ramp_time = 0.f;

        float m_current_right_lerp_value = 0.f;
        float m_last_right_lerp_value = 0.f;
        float m_right_ramp_time = 0.f;
        float m_right_release_ramp_time = 0.f;

        float m_current_sprint_lerp_value = 1.f;
        float m_last_sprint_lerp_value = 1.f;
        float m_sprint_ramp_time = 0.f;
        float m_sprint_release_ramp_time = 0.f;

        // Track when the key is pressed / released
        bool m_forward_pressed = false;
        bool m_back_pressed = false;
        bool m_left_pressed = false;
        bool m_right_pressed = false;
        bool m_sprint_pressed = false;

        // Event value multipliers
        float m_forward_value = 0.f;
        float m_back_value = 0.f;
        float m_left_value = 0.f;
        float m_right_value = 0.f;
        float m_yaw_value = 0.f;
        float m_pitch_value = 0.f;
        float m_sprint_value = 1.f;

        bool* m_pressed[5] = {&m_forward_pressed, &m_back_pressed, &m_left_pressed, &m_right_pressed, &m_sprint_pressed};

        float* m_directions_lerp[5][5] = {
            {&m_forward_value, &m_current_forward_lerp_value, &m_last_forward_lerp_value, &m_forward_ramp_time, &m_forward_release_ramp_time},
            {&m_back_value, &m_current_back_lerp_value, &m_last_back_lerp_value, &m_back_ramp_time, &m_back_release_ramp_time},
            {&m_left_value, &m_current_left_lerp_value, &m_last_left_lerp_value, &m_left_ramp_time, &m_left_release_ramp_time},
            {&m_right_value, &m_current_right_lerp_value, &m_last_right_lerp_value, &m_right_ramp_time, &m_right_release_ramp_time},
            {&m_sprint_value, &m_current_sprint_lerp_value, &m_last_sprint_lerp_value, &m_sprint_ramp_time, &m_sprint_release_ramp_time}};

        // Event IDs and action names
        StartingPointInput::InputEventNotificationId m_MoveForwardEventId;
        AZStd::string m_str_forward;
        StartingPointInput::InputEventNotificationId m_MoveBackEventId;
        AZStd::string m_str_back;
        StartingPointInput::InputEventNotificationId m_MoveLeftEventId;
        AZStd::string m_str_left;
        StartingPointInput::InputEventNotificationId m_MoveRightEventId;
        AZStd::string m_str_right;
        StartingPointInput::InputEventNotificationId m_RotateYawEventId;
        AZStd::string m_str_yaw;
        StartingPointInput::InputEventNotificationId m_RotatePitchEventId;
        AZStd::string m_str_pitch;
        StartingPointInput::InputEventNotificationId m_SprintEventId;
        AZStd::string m_str_sprint;

        // list of action names
        AZStd::string* m_input_names[7] = {
            &m_str_forward, &m_str_back,
            &m_str_left, &m_str_right,
            &m_str_yaw, &m_str_pitch,
            &m_str_sprint
        };

        // map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_control_map = {
            {&m_MoveForwardEventId, &m_forward_value},
            {&m_MoveBackEventId, &m_back_value},
            {&m_MoveLeftEventId, &m_left_value},
            {&m_MoveRightEventId, &m_right_value},
            {&m_RotateYawEventId, &m_yaw_value},
            {&m_RotatePitchEventId, &m_pitch_value},
            {&m_SprintEventId, &m_sprint_value}};
    };
}
