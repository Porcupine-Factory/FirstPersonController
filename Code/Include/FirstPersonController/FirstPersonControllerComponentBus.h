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
        virtual float GetZVelocity() const = 0;
        virtual void SetZVelocity(const float&) = 0;
        virtual float GetJumpInitialVelocity() const = 0;
        virtual void SetJumpInitialVelocity(const float&) = 0;
        virtual bool GetDoubleJump() const = 0;
        virtual void SetDoubleJump(const bool&) = 0;
        virtual float GetGroundedOffset() const = 0;
        virtual void SetGroundedOffset(const float&) = 0;
        virtual float GetJumpHoldOffset() const = 0;
        virtual void SetJumpHoldOffset(const float&) = 0;
        virtual float GetSphereCastRadiusPercentageIncrease() const = 0;
        virtual void SetSphereCastRadiusPercentageIncrease(const float&) = 0;
        virtual float GetMaxGroundedAngleDegrees() const = 0;
        virtual void SetMaxGroundedAngleDegrees(const float&) = 0;
        virtual float GetTopWalkSpeed() const = 0;
        virtual void SetTopWalkSpeed(const float&) = 0;
        virtual float GetWalkAcceleration() const = 0;
        virtual void SetWalkAcceleration(const float&) = 0;
        virtual float GetWalkDeceleration() const = 0;
        virtual void SetWalkDeceleration(const float&) = 0;
        virtual float GetOpposingDecel() const = 0;
        virtual void SetOpposingDecel(const float&) = 0;
        virtual bool GetInstantVelocityRotation() const = 0;
        virtual void SetInstantVelocityRotation(const bool&) = 0;
        virtual float GetSprintVelocityScale() const = 0;
        virtual void SetSprintVelocityScale(const float&) = 0;
        virtual float GetSprintAccelScale() const = 0;
        virtual void SetSprintAccelScale(const float&) = 0;
        virtual float GetCrouchScale() const = 0;
        virtual void SetCrouchScale(const float&) = 0;
        virtual float GetSprintMaxTime() const = 0;
        virtual void SetSprintMaxTime(const float&) = 0;
        virtual float GetSprintHeldTime() const = 0;
        virtual void SetSprintHeldTime(const float&) = 0;
        virtual float GetStaminaPercentage() const = 0 ;
        virtual void SetStaminaPercentage(const float&) = 0;
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
        virtual float GetUncrouchHeadSphereCastOffset() const = 0;
        virtual void SetUncrouchHeadSphereCastOffset(const float&) = 0;
        virtual bool GetCrouchEnableToggle() const = 0;
        virtual void SetCrouchEnableToggle(const bool&) = 0;
        virtual bool GetCrouchJumpCausesStanding() const = 0;
        virtual void SetCrouchJumpCausesStanding(const bool&) = 0;
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
        virtual void OnCrouched() = 0;
        virtual void OnStoodUp() = 0;
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
            AZ::SystemAllocator, OnGroundHit, OnGroundSoonHit, OnUngrounded, OnCrouched, OnStoodUp, OnFirstJump, OnSecondJump, OnSprintCooldown);

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
        void OnCrouched() override
        {
            Call(FN_OnCrouched);
        }
        void OnStoodUp() override
        {
            Call(FN_OnStoodUp);
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
