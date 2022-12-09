#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/std/containers/map.h>

#include <StartingPointInput/InputEventNotificationBus.h>

#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>

namespace FirstPersonController
{
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

        static void GetRequredServices(AZ::ComponentDescriptor::DependencyArrayType& req);

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

    private:
        AZ::Entity* m_activeCameraEntity = nullptr;
        AZ::Entity* GetActiveCamera();

        // Child entity IDs
        bool m_obtained_child_ids = false;
        AZStd::vector<AZ::EntityId> m_children;

        void ProcessInput(const float& deltaTime);

        void UpdateVelocityXY(const float& deltaTime);
        void UpdateVelocityZ(const float& deltaTime);

        AZ::Vector3 LerpVelocity(const AZ::Vector3& target_velocity, const float& deltaTime);
        void SlerpRotation(const float& deltaTime);
        void SprintManager(const AZ::Vector3& target_velocity, const float& deltaTime);

        bool CheckGrounded();

        AZ::Vector3 m_apply_velocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_prev_target_velocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_last_applied_velocity = AZ::Vector3::CreateZero();

        float m_speed = 10.f;
        
        float m_lerp_time = 0.f;

        // Jumping and gravity
        float m_gravity = -9.81f;
        bool m_grounded = true;
        bool m_ground_close = true;
        bool m_jump_pressed = false;
        bool m_jump_held = false;
        float m_z_velocity = 0.f;
        float m_capsule_radius = 0.25f;
        float m_capsule_height = 1.753f;
        // The capsule offset determines how far below the character's feet the ground is detected
        float m_capsule_offset = 0.001f;
        // The capsule jump hold offset makes it so that the initial jump velocity is held constant
        // for the offset value entered, up to a maximum of roughly the capsule height,
        // depending on the jump velocity/height
        float m_capsule_jump_hold_offset = 0.8765f;

        void UpdateRotation(const float& deltaTime);
        // These default values work well, depending on OS mouse settings,
        // assuming the event value multiplier is 1.0
        float m_yaw_sensitivity = 0.005f;
        float m_pitch_sensitivity = 0.005f;

        float m_current_heading = 0.f;
        AZ::Quaternion m_new_look_direction = AZ::Quaternion::CreateZero();
        float m_rotation_damp = 20.f;

        // Acceleration lerp movement
        float m_accel = 1.f;

        // Deceleration factor
        float m_decel = 1.414f;
        float m_break = 1.414f;

        // Movement scale factors
        // assuming the event value multipliers are all +/-1.0
        float m_forward_scale = 1.f;
        float m_back_scale = 0.75f;
        float m_left_scale = 1.f;
        float m_right_scale = 1.f;

        // Event value multipliers
        float m_forward_value = 0.f;
        float m_back_value = 0.f;
        float m_left_value = 0.f;
        float m_right_value = 0.f;
        float m_yaw_value = 0.f;
        float m_pitch_value = 0.f;
        float m_sprint_value = 1.f;
        float m_jump_value = 0.f;

        // Sprint application variables
        float m_sprint_pressed_value = 1.f;
        float m_sprint_velocity_adjust = 0.f;
        float m_sprint_accel_adjust = 0.f;
        float m_sprint_time = 0.f;
        float m_sprint_held_duration = 0.f;
        float m_sprint_max_time = 3.f;
        float m_sprint_cooldown = 0.f;
        float m_sprint_cooldown_time = 5.f;

        // Event IDs and action names
        StartingPointInput::InputEventNotificationId m_MoveForwardEventId;
        AZStd::string m_str_forward = "Forward";
        StartingPointInput::InputEventNotificationId m_MoveBackEventId;
        AZStd::string m_str_back = "Back";
        StartingPointInput::InputEventNotificationId m_MoveLeftEventId;
        AZStd::string m_str_left = "Left";
        StartingPointInput::InputEventNotificationId m_MoveRightEventId;
        AZStd::string m_str_right = "Right";
        StartingPointInput::InputEventNotificationId m_RotateYawEventId;
        AZStd::string m_str_yaw = "Yaw";
        StartingPointInput::InputEventNotificationId m_RotatePitchEventId;
        AZStd::string m_str_pitch = "Pitch";
        StartingPointInput::InputEventNotificationId m_SprintEventId;
        AZStd::string m_str_sprint = "Sprint";
        StartingPointInput::InputEventNotificationId m_JumpEventId;
        AZStd::string m_str_jump = "Jump";

        // list of action names
        AZStd::string* m_input_names[8] = {
            &m_str_forward, &m_str_back,
            &m_str_left, &m_str_right,
            &m_str_yaw, &m_str_pitch,
            &m_str_sprint, &m_str_jump
        };

        // map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_control_map = {
            {&m_MoveForwardEventId, &m_forward_value},
            {&m_MoveBackEventId, &m_back_value},
            {&m_MoveLeftEventId, &m_left_value},
            {&m_MoveRightEventId, &m_right_value},
            {&m_RotateYawEventId, &m_yaw_value},
            {&m_RotatePitchEventId, &m_pitch_value},
            {&m_SprintEventId, &m_sprint_value},
            {&m_JumpEventId, &m_jump_value}};
    };
}
