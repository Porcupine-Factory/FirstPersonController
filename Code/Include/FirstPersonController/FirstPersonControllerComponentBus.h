#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class FirstPersonControllerComponentRequests : public AZ::ComponentBus
    {
    public:
        ~FirstPersonControllerComponentRequests() override = default;

        virtual AZ::EntityId GetActiveCameraId() const = 0;
        virtual bool GetGrounded() const = 0;
        virtual void SetGroundedForTick(const bool&) = 0;
        virtual bool GetGroundClose() const = 0;
        virtual void SetGroundCloseForTick(const bool&) = 0;
        virtual float GetAirTime() const = 0;
        virtual float GetJumpKeyValue() const = 0;
        virtual float GetGravity() const = 0;
        virtual void SetGravity(const float&) = 0;
        virtual float GetInitialJumpVelocity() const = 0;
        virtual void SetInitialJumpVelocity(const float&) = 0;
        virtual bool GetDoubleJump() const = 0;
        virtual void SetDoubleJump(const bool&) = 0;
        virtual float GetCapsuleOffset() const = 0;
        virtual void SetCapsuleOffset(const float&) = 0;
        virtual float GetCapsuleJumpHoldOffset() const = 0;
        virtual void SetCapsuleJumpHoldOffset(const float&) = 0;
        virtual float GetMaxGroundedAngleDegrees() const = 0;
        virtual void SetMaxGroundedAngleDegrees(const float&) = 0;
        virtual float GetTopWalkSpeed() const = 0;
        virtual void SetTopWalkSpeed(const float&) = 0;
        virtual float GetWalkAcceleration() const = 0;
        virtual void SetWalkAcceleration(const float&) = 0;
        virtual float GetWalkDeceleration() const = 0;
        virtual void SetWalkDeceleration(const float&) = 0;
        virtual float GetWalkBreak() const = 0;
        virtual void SetWalkBreak(const float&) = 0;
        virtual float GetSprintScale() const = 0;
        virtual void SetSprintScale(const float&) = 0;
        virtual float GetSprintHeldTime() const = 0;
        virtual void SetSprintHeldTime(const float&) = 0;
        virtual float GetSprintCooldown() const = 0;
        virtual void SetSprintCooldown(const float&) = 0;
        virtual float GetSprintPauseTime() const = 0;
        virtual void SetSprintPauseTime(const float&) = 0;
        virtual bool GetCrouching() const = 0;
        virtual void SetCrouching(const bool&) = 0;
        virtual float GetCrouchDistance() const = 0;
        virtual void SetCrouchDistance(const float&) = 0;
        virtual float GetCrouchTime() const = 0;
        virtual void SetCrouchTime(const float&) = 0;
        virtual bool GetCrouchEnableToggle() const = 0;
        virtual void SetCrouchEnableToggle(const bool&) = 0;
        virtual bool GetCrouchSprintCausesStanding() const = 0;
        virtual void SetCrouchSprintCausesStanding(const bool&) = 0;
        virtual bool GetCrouchPriorityWhenSprintPressed() const = 0;
        virtual void SetCrouchPriorityWhenSprintPressed(const bool&) = 0;
        virtual float GetCameraPitchSensitivity() const = 0;
        virtual void SetCameraPitchSensitivity(const float&) = 0;
        virtual float GetCameraYawSensitivity() const = 0;
        virtual void SetCameraYawSensitivity(const float&) = 0;
        virtual float GetCameraRotationDampFactor() const = 0;
        virtual void SetCameraRotationDampFactor(const float&) = 0;
        virtual void UpdateCameraPitch(const float&) = 0;
        virtual void UpdateCameraYaw(const float&) = 0;
        virtual float GetHeading() const = 0;
    };

    using FirstPersonControllerComponentRequestBus = AZ::EBus<FirstPersonControllerComponentRequests>;

    class FirstPersonControllerNotifications
        : public AZ::ComponentBus
    {
    public:
        virtual void OnGroundHit() = 0;
        virtual void OnGroundSoonHit() = 0;
        virtual void OnUngrounded() = 0;
        virtual void OnFirstJump() = 0;
        virtual void OnSecondJump() = 0;
        virtual void OnSprintCooldown() = 0;
    };

    using FirstPersonControllerNotificationBus = AZ::EBus<FirstPersonControllerNotifications>;

    class FirstPersonControllerNotificationHandler
        : public FirstPersonControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(FirstPersonControllerNotificationHandler,
            "{b6d9e703-2c1b-4282-81a9-249123f3eee8}",
            AZ::SystemAllocator, OnGroundHit, OnGroundSoonHit, OnUngrounded, OnFirstJump, OnSecondJump, OnSprintCooldown);

        void OnGroundHit() override
        {
            Call(FN_OnGroundHit);
        }
        void OnGroundSoonHit() override
        {
            Call(FN_OnGroundSoonHit);
        }
        void OnUngrounded() override
        {
            Call(FN_OnUngrounded);
        }
        void OnFirstJump() override
        {
            Call(FN_OnFirstJump);
        }
        void OnSecondJump() override
        {
            Call(FN_OnSecondJump);
        }
        void OnSprintCooldown() override
        {
            Call(FN_OnSprintCooldown);
        }
    };
} // namespace FirstPersonController
