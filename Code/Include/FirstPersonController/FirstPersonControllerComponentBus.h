#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Math/Vector3.h>

#include <AzFramework/Physics/PhysicsScene.h>

namespace FirstPersonController
{
    class FirstPersonControllerComponentRequests : public AZ::ComponentBus
    {
    public:
        ~FirstPersonControllerComponentRequests() override = default;

        virtual AZ::Entity* GetActiveCameraEntityPtr() const = 0;
        virtual AZ::EntityId GetActiveCameraEntityId() const = 0;
        virtual void ReacquireChildEntityIds() = 0;
        virtual void ReacquireCapsuleDimensions() = 0;
        virtual void ReacquireMaxSlopeAngle() = 0;
        virtual AZStd::string GetForwardEventName() const = 0;
        virtual void SetForwardEventName(const AZStd::string&) = 0;
        virtual float GetForwardScale() const = 0;
        virtual void SetForwardScale(const float&) = 0;
        virtual float GetForwardInputValue() const = 0;
        virtual void SetForwardInputValue(const float&) = 0;
        virtual AZStd::string GetBackEventName() const = 0;
        virtual void SetBackEventName(const AZStd::string&) = 0;
        virtual float GetBackScale() const = 0;
        virtual void SetBackScale(const float&) = 0;
        virtual float GetBackInputValue() const = 0;
        virtual void SetBackInputValue(const float&) = 0;
        virtual AZStd::string GetLeftEventName() const = 0;
        virtual void SetLeftEventName(const AZStd::string&) = 0;
        virtual float GetLeftScale() const = 0;
        virtual void SetLeftScale(const float&) = 0;
        virtual float GetLeftInputValue() const = 0;
        virtual void SetLeftInputValue(const float&) = 0;
        virtual AZStd::string GetRightEventName() const = 0;
        virtual void SetRightEventName(const AZStd::string&) = 0;
        virtual float GetRightScale() const = 0;
        virtual void SetRightScale(const float&) = 0;
        virtual float GetRightInputValue() const = 0;
        virtual void SetRightInputValue(const float&) = 0;
        virtual AZStd::string GetYawEventName() const = 0;
        virtual void SetYawEventName(const AZStd::string&) = 0;
        virtual float GetYawInputValue() const = 0;
        virtual void SetYawInputValue(const float&) = 0;
        virtual AZStd::string GetPitchEventName() const = 0;
        virtual void SetPitchEventName(const AZStd::string&) = 0;
        virtual float GetPitchInputValue() const = 0;
        virtual void SetPitchInputValue(const float&) = 0;
        virtual AZStd::string GetSprintEventName() const = 0;
        virtual void SetSprintEventName(const AZStd::string&) = 0;
        virtual float GetSprintInputValue() const = 0;
        virtual void SetSprintInputValue(const float&) = 0;
        virtual AZStd::string GetCrouchEventName() const = 0;
        virtual void SetCrouchEventName(const AZStd::string&) = 0;
        virtual float GetCrouchInputValue() const = 0;
        virtual void SetCrouchInputValue(const float&) = 0;
        virtual AZStd::string GetJumpEventName() const = 0;
        virtual void SetJumpEventName(const AZStd::string&) = 0;
        virtual float GetJumpInputValue() const = 0;
        virtual void SetJumpInputValue(const float&) = 0;
        virtual bool GetGrounded() const = 0;
        virtual void SetGroundedForTick(const bool&) = 0;
        virtual AZStd::vector<AzPhysics::SceneQueryHit> GetGroundSceneQueryHits() const = 0;
        virtual AZStd::vector<AzPhysics::SceneQueryHit> GetGroundCloseSceneQueryHits() const = 0;
        virtual AZ::Vector3 GetGroundSumNormalsDirection() const = 0;
        virtual AZ::Vector3 GetGroundCloseSumNormalsDirection() const = 0;
        virtual AZ::EntityId GetSceneQueryHitEntityId(AzPhysics::SceneQueryHit) const = 0;
        virtual AZ::Vector3 GetSceneQueryHitNormal(AzPhysics::SceneQueryHit) const = 0;
        virtual AZ::Vector3 GetSceneQueryHitPosition(AzPhysics::SceneQueryHit) const = 0;
        virtual float GetSceneQueryHitDistance(AzPhysics::SceneQueryHit) const = 0;
        virtual Physics::MaterialId GetSceneQueryHitMaterialId(AzPhysics::SceneQueryHit) const = 0;
        virtual Physics::Shape* GetSceneQueryHitShapePtr(AzPhysics::SceneQueryHit) const = 0;
        virtual AzPhysics::SimulatedBodyHandle GetSceneQueryHitSimulatedBodyHandle(AzPhysics::SceneQueryHit) const = 0;
        virtual bool GetGroundClose() const = 0;
        virtual void SetGroundCloseForTick(const bool&) = 0;
        virtual AZStd::string GetGroundedCollisionGroupName() const = 0;
        virtual void SetGroundedCollisionGroup(const AZStd::string&) = 0;
        virtual float GetAirTime() const = 0;
        virtual float GetGravity() const = 0;
        virtual void SetGravity(const float&) = 0;
        virtual AZ::Vector3 GetPrevTargetVelocityWorld() const = 0;
        virtual AZ::Vector3 GetPrevTargetVelocityHeading() const = 0;
        virtual float GetVelocityCloseTolerance() const = 0;
        virtual void SetVelocityCloseTolerance(const float&) = 0;
        virtual AZ::Vector3 TiltVectorXCrossY(const AZ::Vector2, const AZ::Vector3&) = 0;
        virtual AZ::Vector3 GetVelocityXCrossYDirection() const = 0;
        virtual void SetVelocityXCrossYDirection(const AZ::Vector3&) = 0;
        virtual bool GetVelocityXCrossYTracksNormal() const = 0;
        virtual void SetVelocityXCrossYTracksNormal(const bool&) = 0;
        virtual AZ::Vector3 GetVelocityZPosDirection() const = 0;
        virtual void SetVelocityZPosDirection(const AZ::Vector3&) = 0;
        virtual AZ::Vector3 GetSphereCastsAxisDirectionPose() const = 0;
        virtual void SetSphereCastsAxisDirectionPose(const AZ::Vector3&) = 0;
        virtual AZ::Vector3 GetVectorAnglesBetweenVectorsRadians(const AZ::Vector3&, const AZ::Vector3&) = 0;
        virtual AZ::Vector3 GetVectorAnglesBetweenVectorsDegrees(const AZ::Vector3&, const AZ::Vector3&) = 0;
        virtual AZ::Vector2 CreateEllipseScaledVector(const AZ::Vector2&, float, float, float, float) = 0;
        virtual float GetJumpHeldGravityFactor() const = 0;
        virtual void SetJumpHeldGravityFactor(const float&) = 0;
        virtual float GetJumpFallingGravityFactor() const = 0;
        virtual void SetJumpFallingGravityFactor(const float&) = 0;
        virtual float GetJumpAccelFactor() const = 0;
        virtual void SetJumpAccelFactor(const float&) = 0;
        virtual bool GetUpdateXYAscending() const = 0;
        virtual void SetUpdateXYAscending(const bool&) = 0;
        virtual bool GetUpdateXYDescending() const = 0;
        virtual void SetUpdateXYDescending(const bool&) = 0;
        virtual bool GetUpdateXYOnlyNearGround() const = 0;
        virtual void SetUpdateXYOnlyNearGround(const bool&) = 0;
        virtual bool GetAddVelocityForTimestepVsTick() const = 0;
        virtual void SetAddVelocityForTimestepVsTick(const bool&) = 0;
        virtual float GetPhysicsTimestepScaleFactor() const = 0;
        virtual void SetPhysicsTimestepScaleFactor(const float&) = 0;
        virtual bool GetScriptSetsTargetVelocityXY() const = 0;
        virtual void SetScriptSetsTargetVelocityXY(const bool&) = 0;
        virtual AZ::Vector2 GetTargetVelocityXY() const = 0;
        virtual void SetTargetVelocityXY(const AZ::Vector2&) = 0;
        virtual AZ::Vector2 GetCorrectedVelocityXY() const = 0;
        virtual void SetCorrectedVelocityXY(const AZ::Vector2&) = 0;
        virtual float GetCorrectedVelocityZ() const = 0;
        virtual void SetCorrectedVelocityZ(const float&) = 0;
        virtual AZ::Vector2 GetApplyVelocityXY() const = 0;
        virtual void SetApplyVelocityXY(const AZ::Vector2&) = 0;
        virtual AZ::Vector3 GetAddVelocityWorld() const = 0;
        virtual void SetAddVelocityWorld(const AZ::Vector3&) = 0;
        virtual AZ::Vector3 GetAddVelocityHeading() const = 0;
        virtual void SetAddVelocityHeading(const AZ::Vector3&) = 0;
        virtual float GetApplyVelocityZ() const = 0;
        virtual void SetApplyVelocityZ(const float&) = 0;
        virtual float GetJumpInitialVelocity() const = 0;
        virtual void SetJumpInitialVelocity(const float&) = 0;
        virtual float GetJumpSecondInitialVelocity() const = 0;
        virtual void SetJumpSecondInitialVelocity(const float&) = 0;
        virtual bool GetJumpReqRepress() const = 0;
        virtual void SetJumpReqRepress(const bool&) = 0;
        virtual bool GetJumpHeld() const = 0;
        virtual void SetJumpHeld(const bool&) = 0;
        virtual bool GetDoubleJump() const = 0;
        virtual void SetDoubleJump(const bool&) = 0;
        virtual float GetGroundedOffset() const = 0;
        virtual void SetGroundedOffset(const float&) = 0;
        virtual float GetGroundCloseOffset() const = 0;
        virtual void SetGroundCloseOffset(const float&) = 0;
        virtual float GetJumpHoldDistance() const = 0;
        virtual void SetJumpHoldDistance(const float&) = 0;
        virtual float GetJumpHeadSphereCastOffset() const = 0;
        virtual void SetJumpHeadSphereCastOffset(const float&) = 0;
        virtual bool GetHeadHitSetsApogee() const = 0;
        virtual void SetHeadHitSetsApogee(const bool&) = 0;
        virtual bool GetHeadHit() const = 0;
        virtual void SetHeadHit(const bool&) = 0;
        virtual bool GetJumpHeadIgnoreNonKinematicRigidBodies() const = 0;
        virtual void SetJumpHeadIgnoreNonKinematicRigidBodies(const bool&) = 0;
        virtual AZStd::string GetHeadCollisionGroupName() const = 0;
        virtual void SetHeadCollisionGroup(const AZStd::string&) = 0;
        virtual AzPhysics::SceneQuery::ResultFlags GetSceneQueryHitResultFlags(AzPhysics::SceneQueryHit) const = 0;
        virtual AZStd::vector<AZ::EntityId> GetHeadHitEntityIds() const = 0;
        virtual float GetGroundSphereCastsRadiusPercentageIncrease() const = 0;
        virtual void SetGroundSphereCastsRadiusPercentageIncrease(const float&) = 0;
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
        virtual bool GetAccelerating() const = 0;
        virtual bool GetDecelerationFactorApplied() const = 0;
        virtual bool GetOpposingDecelFactorApplied() const = 0;
        virtual bool GetInstantVelocityRotation() const = 0;
        virtual void SetInstantVelocityRotation(const bool&) = 0;
        virtual bool GetVelocityIgnoresObstacles() const = 0;
        virtual void SetVelocityIgnoresObstacles(const bool&) = 0;
        virtual bool GetHitSomething() const = 0;
        virtual void SetHitSomething(const bool&) = 0;
        virtual float GetSprintScaleForward() const = 0;
        virtual void SetSprintScaleForward(const float&) = 0;
        virtual float GetSprintScaleBack() const = 0;
        virtual void SetSprintScaleBack(const float&) = 0;
        virtual float GetSprintScaleLeft() const = 0;
        virtual void SetSprintScaleLeft(const float&) = 0;
        virtual float GetSprintScaleRight() const = 0;
        virtual void SetSprintScaleRight(const float&) = 0;
        virtual float GetSprintAccelScale() const = 0;
        virtual void SetSprintAccelScale(const float&) = 0;
        virtual float GetSprintMaxTime() const = 0;
        virtual void SetSprintMaxTime(const float&) = 0;
        virtual float GetSprintHeldTime() const = 0;
        virtual void SetSprintHeldTime(const float&) = 0;
        virtual float GetSprintRegenRate() const = 0;
        virtual void SetSprintRegenRate(const float&) = 0;
        virtual float GetStaminaPercentage() const = 0;
        virtual void SetStaminaPercentage(const float&) = 0;
        virtual bool GetStaminaIncreasing() const = 0;
        virtual bool GetStaminaDecreasing() const = 0;
        virtual bool GetSprintUsesStamina() const = 0;
        virtual void SetSprintUsesStamina(const bool&) = 0;
        virtual bool GetRegenerateStaminaAutomatically() const = 0;
        virtual void SetRegenerateStaminaAutomatically(const bool&) = 0;
        virtual bool GetSprinting() const = 0;
        virtual float GetSprintCooldownTime() const = 0;
        virtual void SetSprintCooldownTime(const float&) = 0;
        virtual float GetSprintCooldown() const = 0;
        virtual void SetSprintCooldown(const float&) = 0;
        virtual float GetSprintPauseTime() const = 0;
        virtual void SetSprintPauseTime(const float&) = 0;
        virtual float GetSprintPause() const = 0;
        virtual void SetSprintPause(const float&) = 0;
        virtual bool GetSprintBackwards() const = 0;
        virtual void SetSprintBackwards(const bool&) = 0;
        virtual bool GetSprintWhileCrouched() const = 0;
        virtual void SetSprintWhileCrouched(const bool&) = 0;
        virtual bool GetCrouching() const = 0;
        virtual void SetCrouching(const bool&) = 0;
        virtual bool GetCrouched() const = 0;
        virtual bool GetStanding() const = 0;
        virtual float GetCrouchedPercentage() const = 0;
        virtual bool GetCrouchScriptLocked() const = 0;
        virtual void SetCrouchScriptLocked(const bool&) = 0;
        virtual float GetCrouchScale() const = 0;
        virtual void SetCrouchScale(const float&) = 0;
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
        virtual bool GetSprintViaScript() const = 0;
        virtual void SetSprintViaScript(const bool&) = 0;
        virtual bool GetSprintEnableDisableScript() const = 0;
        virtual void SetSprintEnableDisableScript(const bool&) = 0;
        virtual float GetCameraYawSensitivity() const = 0;
        virtual void SetCameraYawSensitivity(const float&) = 0;
        virtual float GetCameraPitchSensitivity() const = 0;
        virtual void SetCameraPitchSensitivity(const float&) = 0;
        virtual float GetCameraPitchMaxAngleRadians() const = 0;
        virtual void SetCameraPitchMaxAngleRadians(const float&) = 0;
        virtual float GetCameraPitchMaxAngleDegrees() const = 0;
        virtual void SetCameraPitchMaxAngleDegrees(const float&) = 0;
        virtual float GetCameraPitchMinAngleRadians() const = 0;
        virtual void SetCameraPitchMinAngleRadians(const float&) = 0;
        virtual float GetCameraPitchMinAngleDegrees() const = 0;
        virtual void SetCameraPitchMinAngleDegrees(const float&) = 0;
        virtual float GetCameraRotationDampFactor() const = 0;
        virtual void SetCameraRotationDampFactor(const float&) = 0;
        virtual bool GetCameraSlerpInsteadOfLerpRotation() const = 0;;
        virtual void SetCameraSlerpInsteadOfLerpRotation(const bool&) = 0;;
        virtual bool GetUpdateCameraYawIgnoresInput() const = 0;
        virtual void SetUpdateCameraYawIgnoresInput(const bool&) = 0;
        virtual bool GetUpdateCameraPitchIgnoresInput() const = 0;
        virtual void SetUpdateCameraPitchIgnoresInput(const bool&) = 0;
        virtual void UpdateCameraPitch(const float&) = 0;
        virtual void UpdateCameraYaw(const float&) = 0;
        virtual float GetHeading() const = 0;
        virtual float GetPitch() const = 0;
    };

    using FirstPersonControllerComponentRequestBus = AZ::EBus<FirstPersonControllerComponentRequests>;

    class FirstPersonControllerNotifications
        : public AZ::ComponentBus
    {
    public:
        virtual void OnGroundHit() = 0;
        virtual void OnGroundSoonHit() = 0;
        virtual void OnUngrounded() = 0;
        virtual void OnStartedFalling() = 0;
        virtual void OnStartedMoving() = 0;
        virtual void OnTargetVelocityReached() = 0;
        virtual void OnStopped() = 0;
        virtual void OnTopWalkSpeedReached() = 0;
        virtual void OnTopSprintSpeedReached() = 0;
        virtual void OnHeadHit() = 0;
        virtual void OnHitSomethingOnXY() = 0;
        virtual void OnCrouched() = 0;
        virtual void OnStoodUp() = 0;
        virtual void OnStartedCrouching() = 0;
        virtual void OnStartedStanding() = 0;
        virtual void OnFirstJump() = 0;
        virtual void OnSecondJump() = 0;
        virtual void OnStaminaCapped() = 0;
        virtual void OnStaminaReachedZero() = 0;
        virtual void OnSprintStarted() = 0;
        virtual void OnCooldownStarted() = 0;
        virtual void OnCooldownDone() = 0;
    };

    using FirstPersonControllerNotificationBus = AZ::EBus<FirstPersonControllerNotifications>;

    class FirstPersonControllerNotificationHandler
        : public FirstPersonControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(FirstPersonControllerNotificationHandler,
            "{b6d9e703-2c1b-4282-81a9-249123f3eee8}",
            AZ::SystemAllocator, OnGroundHit, OnGroundSoonHit, OnUngrounded, OnStartedFalling, OnStartedMoving, OnTargetVelocityReached, OnStopped, OnTopWalkSpeedReached, OnTopSprintSpeedReached, OnHeadHit, OnHitSomethingOnXY, OnCrouched, OnStoodUp, OnStartedCrouching, OnStartedStanding, OnFirstJump, OnSecondJump, OnStaminaCapped, OnStaminaReachedZero, OnSprintStarted, OnCooldownStarted, OnCooldownDone);

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
        void OnStartedFalling() override
        {
            Call(FN_OnStartedFalling);
        }
        void OnStartedMoving()
        {
            Call(FN_OnStartedMoving);
        }
        void OnTargetVelocityReached()
        {
            Call(FN_OnTargetVelocityReached);
        }
        void OnStopped()
        {
            Call(FN_OnStopped);
        }
        void OnTopWalkSpeedReached()
        {
            Call(FN_OnTopWalkSpeedReached);
        }
        void OnTopSprintSpeedReached()
        {
            Call(FN_OnTopSprintSpeedReached);
        }
        void OnHeadHit() override
        {
            Call(FN_OnHeadHit);
        }
        void OnHitSomethingOnXY() override
        {
            Call(FN_OnHitSomethingOnXY);
        }
        void OnCrouched() override
        {
            Call(FN_OnCrouched);
        }
        void OnStoodUp() override
        {
            Call(FN_OnStoodUp);
        }
        void OnStartedCrouching() override
        {
            Call(FN_OnStartedCrouching);
        }
        void OnStartedStanding() override
        {
            Call(FN_OnStartedStanding);
        }
        void OnFirstJump() override
        {
            Call(FN_OnFirstJump);
        }
        void OnSecondJump() override
        {
            Call(FN_OnSecondJump);
        }
        void OnStaminaCapped() override
        {
            Call(FN_OnStaminaCapped);
        }
        void OnStaminaReachedZero() override
        {
            Call(FN_OnStaminaReachedZero);
        }
        void OnSprintStarted() override
        {
            Call(FN_OnSprintStarted);
        }
        void OnCooldownStarted() override
        {
            Call(FN_OnCooldownStarted);
        }
        void OnCooldownDone() override
        {
            Call(FN_OnCooldownDone);
        }
    };
} // namespace FirstPersonController
