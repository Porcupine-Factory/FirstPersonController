#pragma once
#include <FirstPersonController/FirstPersonControllerComponentBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/std/containers/map.h>

#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>

#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    class FirstPersonControllerComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::InputChannelEventListener
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
        , public FirstPersonControllerComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FirstPersonControllerComponent, "{0a47c7c2-0f94-48dd-8e3f-fd55c30475b9}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

        // AzFramework::InputChannelEventListener
        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        // Gamepad Events
        void OnGamepadEvent(const AzFramework::InputChannel& inputChannel);

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // FirstPersonControllerRequestBus
        AZ::EntityId GetActiveCameraId() const;
        bool GetGrounded() const override;
        void SetGroundedForTick(const bool& new_grounded) override;
        bool GetGroundClose() const override;
        void SetGroundCloseForTick(const bool& new_groundClose) override;
        float GetAirTime() const override;
        float GetJumpKeyValue() const override;;
        float GetGravity() const override;
        void SetGravity(const float& new_gravity) override;
        float GetZVelocity() const override;
        void SetZVelocity(const float& new_zVelocity) override;
        float GetJumpInitialVelocity() const override;
        void SetJumpInitialVelocity(const float& new_jumpInitialVelocity) override;
        bool GetDoubleJump() const override;
        void SetDoubleJump(const bool& new_doubleJumpEnabled) override;
        float GetGroundedOffset() const override;
        void SetGroundedOffset(const float& new_sphereCastJumpHoldOffset) override;
        float GetJumpHoldOffset() const override;
        void SetJumpHoldOffset(const float& new_groundedSphereCastOffset) override;
        float GetMaxGroundedAngleDegrees() const override;
        void SetMaxGroundedAngleDegrees(const float& new_maxGroundedAngleDegrees) override;
        float GetTopWalkSpeed() const override;
        void SetTopWalkSpeed(const float& new_speed) override;
        float GetWalkAcceleration() const override;
        void SetWalkAcceleration(const float& new_accel) override;
        float GetWalkDeceleration() const override;
        void SetWalkDeceleration(const float& new_decel) override;
        float GetWalkBreak() const override;
        void SetWalkBreak(const float& new_break) override;
        float GetSprintScale() const override;
        void SetSprintScale(const float& new_sprintScale) override;
        float GetCrouchScale() const override;
        void SetCrouchScale(const float& new_crouchScale) override;
        float GetSprintMaxTime() const override;
        void SetSprintMaxTime(const float& new_sprintMaxTime) override;
        float GetSprintHeldTime() const override;
        void SetSprintHeldTime(const float& new_sprintHeldDuration) override;
        float GetSprintCooldown() const override;
        void SetSprintCooldown(const float& new_sprintCooldown) override;
        float GetSprintPauseTime() const override;
        void SetSprintPauseTime(const float& new_sprintDecrementPause) override;
        bool GetCrouching() const override;
        void SetCrouching(const bool& new_crouching) override;
        float GetCrouchDistance() const override;
        void SetCrouchDistance(const float& new_crouchDistance) override;
        float GetCrouchTime() const override;
        void SetCrouchTime(const float& new_crouchTime) override;
        float GetUncrouchHeadSphereCastOffset() const override;
        void SetUncrouchHeadSphereCastOffset(const float& new_uncrouchHeadSphereCastOffset) override;
        bool GetCrouchEnableToggle() const override;
        void SetCrouchEnableToggle(const bool& new_crouchEnableToggle) override;
        bool GetCrouchJumpCausesStanding() const override;
        void SetCrouchJumpCausesStanding(const bool& new_crouchJumpCausesStanding) override;
        bool GetCrouchSprintCausesStanding() const override;
        void SetCrouchSprintCausesStanding(const bool& new_crouchSprintCausesStanding) override;
        bool GetCrouchPriorityWhenSprintPressed() const override;
        void SetCrouchPriorityWhenSprintPressed(const bool& new_crouchPriorityWhenSprintPressed) override;
        float GetCameraPitchSensitivity() const override;
        void SetCameraPitchSensitivity(const float& new_pitchSensitivity) override;
        float GetCameraYawSensitivity() const override;
        void SetCameraYawSensitivity(const float& new_yawSensitivity) override;
        float GetCameraRotationDampFactor() const override;
        void SetCameraRotationDampFactor(const float& new_rotationDamp) override;
        void UpdateCameraPitch(const float& new_cameraPitchAngle) override;
        void UpdateCameraYaw(const float& new_cameraYawAngle) override;
        float GetHeading() const override;

    private:
        // Active camera entity
        AZ::Entity* m_activeCameraEntity = nullptr;
        AZ::Entity* GetActiveCamera() const;

        // Child entity IDs
        bool m_obtainedChildIds = false;
        AZStd::vector<AZ::EntityId> m_children;

        // Used to determine if the PhysX Character Controller component's values have been obtained
        bool m_obtainedPhysxCharacterValues = false;

        // Called on each tick
        void ProcessInput(const float& deltaTime);

        // Various methods used to implement the First Person Controller functionality
        void CheckGrounded(const float& deltaTime);
        void UpdateVelocityXY(const float& deltaTime);
        void UpdateJumpMaxHoldTime();
        void UpdateVelocityZ(const float& deltaTime);
        void UpdateRotation(const float& deltaTime);
        AZ::Vector3 LerpVelocity(const AZ::Vector3& targetVelocity, const float& deltaTime);
        void SlerpRotation(const float& deltaTime);
        void SprintManager(const AZ::Vector3& targetVelocity, const float& deltaTime);
        void CrouchManager(const float& deltaTime);

        // FirstPersonControllerNotificationBus
        void OnGroundHit();
        void OnGroundSoonHit();
        void OnUngrounded();
        void OnCrouched();
        void OnStoodUp();
        void OnFirstJump();
        void OnSecondJump();
        void OnSprintCooldown();

        // Velocity application variables
        AZ::Vector3 m_applyVelocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_prevTargetVelocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_lastAppliedVelocity = AZ::Vector3::CreateZero();

        // Angles used to rotate the camera
        float m_cameraRotationAngles[3] = {0.f, 0.f, 0.f};

        // Top walk speed
        float m_speed = 10.f;

        // Used to track where we are along lerping the velocity between the two values
        float m_lerpTime = 0.f;

        // Sprint application variables
        float m_sprintPressedValue = 1.f;
        float m_sprintPrevValue = 1.f;
        float m_sprintVelocityAdjust = 0.f;
        float m_sprintAccelAdjust = 0.f;
        float m_sprintIncrementTime = 0.f;
        float m_sprintHeldDuration = 0.f;
        float m_sprintDecrementPause = 0.f;
        float m_sprintPrevDecrementPause = 0.f;
        float m_sprintMaxTime = 3.f;
        float m_sprintCooldown = 0.f;
        float m_sprintCooldownTime = 5.f;
        bool m_sprintDecrementing = false;

        // Crouch application variables
        float m_crouchDistance = 0.5f;
        float m_crouchTime = 0.2f;
        float m_crouchPrevValue = 0.f;
        bool m_crouching = false;
        bool m_standing = true;
        float m_cameraLocalZTravelDistance = 0.f;
        bool m_crouchEnableToggle = true;
        bool m_crouchJumpCausesStanding = true;
        bool m_crouchSprintCausesStanding = false;
        bool m_crouchPriorityWhenSprintPressed = true;

        // Jumping and gravity
        float m_gravity = -9.81f;
        bool m_grounded = true;
        float m_maxGroundedAngleDegrees = 30.0f;
        bool m_scriptGrounded = true;
        bool m_scriptSetGroundTick = false;
        bool m_groundClose = true;
        bool m_scriptGroundClose = true;
        bool m_scriptSetGroundCloseTick = false;
        float m_airTime = 0.f;
        float m_jumpInitialVelocity = 2.5f;
        bool m_jumpHeld = false;
        bool m_jumpReqRepress = true;
        float m_zVelocity = 0.f;
        float m_zVelocityCurrentDelta = 0.f;
        float m_zVelocityPrevDelta = 0.f;
        float m_capsuleRadius = 0.25f;
        float m_capsuleHeight = 1.753f;
        // The grounded sphere cast offset determines how far below the character's feet the ground is detected
        float m_groundedSphereCastOffset = 0.001f;
        // The sphere cast jump hold offset is used to determine initial (ascending) distance of the of the jump
        // where the m_jumpHeldGravityFactor is applied to the gravity
        // It is also used to determine when the ground is close
        float m_sphereCastJumpHoldOffset = 0.5f;
        float m_jumpMaxHoldTime = m_sphereCastJumpHoldOffset / m_jumpInitialVelocity;
        float m_jumpCounter = 0.f;
        float m_jumpHeldGravityFactor = 0.1f;
        float m_jumpFallingGravityFactor = 1.1f;
        bool m_doubleJumpEnabled = false;
        bool m_secondJump = false;

        // Variables used to determine when the X&Y velocity should be updated
        bool m_updateXYAscending = true;
        bool m_updateXYDecending = true;
        bool m_updateXYOnlyNearGround = true;

        // These default values work well, depending on OS mouse settings,
        // assuming the event value multiplier is 1.0
        float m_pitchSensitivity = 0.005f;
        float m_yawSensitivity = 0.005f;

        // Rotation-related variables
        float m_currentHeading = 0.f;
        AZ::Quaternion m_newLookRotationDelta = AZ::Quaternion::CreateZero();
        float m_rotationDamp = 20.f;

        // Used when a script wants to update the camera angle via the Request Bus or a gamepad
        bool m_rotatingPitchViaScriptGamepad = false;
        bool m_rotatingYawViaScriptGamepad = false;

        // Acceleration lerp movement
        float m_accel = 1.f;
        float m_jumpAccelFactor = 0.5f;

        // Deceleration factor
        float m_decel = 1.414f;
        float m_break = 1.414f;

        // Movement scale factors
        // assuming the event value multipliers are all +/-1.0
        float m_forwardScale = 1.f;
        float m_backScale = 0.75f;
        float m_leftScale = 1.f;
        float m_rightScale = 1.f;
        float m_sprintScale = 1.5f;
        float m_crouchScale = 0.65f;
        // This sphere cast determines how far above the charcter's head that an obstruction is detected
        // for allowing them to uncrouch
        float m_uncrouchHeadSphereCastOffset = 0.1f;

        // Event value multipliers
        float m_forwardValue = 0.f;
        float m_backValue = 0.f;
        float m_leftValue = 0.f;
        float m_rightValue = 0.f;
        float m_yawValue = 0.f;
        float m_pitchValue = 0.f;
        float m_sprintValue = 1.f;
        float m_crouchValue = 0.f;
        float m_jumpValue = 0.f;

        // Event IDs and action names
        StartingPointInput::InputEventNotificationId m_MoveForwardEventId;
        AZStd::string m_strForward = "Forward";
        StartingPointInput::InputEventNotificationId m_MoveBackEventId;
        AZStd::string m_strBack = "Back";
        StartingPointInput::InputEventNotificationId m_MoveLeftEventId;
        AZStd::string m_strLeft = "Left";
        StartingPointInput::InputEventNotificationId m_MoveRightEventId;
        AZStd::string m_strRight = "Right";
        StartingPointInput::InputEventNotificationId m_RotateYawEventId;
        AZStd::string m_strYaw = "Yaw";
        StartingPointInput::InputEventNotificationId m_RotatePitchEventId;
        AZStd::string m_strPitch = "Pitch";
        StartingPointInput::InputEventNotificationId m_SprintEventId;
        AZStd::string m_strSprint = "Sprint";
        StartingPointInput::InputEventNotificationId m_CrouchEventId;
        AZStd::string m_strCrouch = "Crouch";
        StartingPointInput::InputEventNotificationId m_JumpEventId;
        AZStd::string m_strJump = "Jump";

        // list of action names
        AZStd::string* m_inputNames[9] = {
            &m_strForward, &m_strBack,
            &m_strLeft, &m_strRight,
            &m_strYaw, &m_strPitch,
            &m_strSprint, &m_strCrouch,
            &m_strJump
        };

        // map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_controlMap = {
            {&m_MoveForwardEventId, &m_forwardValue},
            {&m_MoveBackEventId, &m_backValue},
            {&m_MoveLeftEventId, &m_leftValue},
            {&m_MoveRightEventId, &m_rightValue},
            {&m_RotateYawEventId, &m_yawValue},
            {&m_RotatePitchEventId, &m_pitchValue},
            {&m_SprintEventId, &m_sprintValue},
            {&m_CrouchEventId, &m_crouchValue},
            {&m_JumpEventId, &m_jumpValue}};
    };
}
