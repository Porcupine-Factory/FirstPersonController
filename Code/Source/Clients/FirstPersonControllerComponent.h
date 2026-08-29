/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#ifdef NETWORKFPC
#include <FirstPersonController/NetworkFPCBotAnimationControllerBus.h>
#include <FirstPersonController/NetworkFPCControllerBus.h>
#endif
#include <FirstPersonController/PidController.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/map.h>

#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>

#include <StartingPointInput/InputEventNotificationBus.h>

#include <PhysXCharacters/API/CharacterController.h>

namespace FirstPersonController
{
    class NetworkFPC;
    class NetworkFPCController;
    class NetworkFPCBotAnimation;
    class NetworkFPCBotAnimationController;

    class FirstPersonControllerComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , protected Physics::CharacterNotificationBus::Handler
#ifdef NETWORKFPC
        , public NetworkFPCControllerNotificationBus::Handler
#endif
        , public AzFramework::InputChannelEventListener
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
        , public FirstPersonControllerComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
        , public Camera::CameraNotificationBus::Handler
    {
        friend class FirstPersonExtrasComponent;
        friend class CameraCoupledChildComponent;
        friend class NetworkFPC;
        friend class NetworkFPCController;
        friend class NetworkFPCBotAnimation;
        friend class NetworkFPCBotAnimationController;

    public:
        AZ_COMPONENT(FirstPersonControllerComponent, "{0a47c7c2-0f94-48dd-8e3f-fd55c30475b9}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::EntityBus interface
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        // Physics::CharacterNotificationBus override
        void OnCharacterActivated(const AZ::EntityId& entityId) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
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
        int GetTickOrder() override;

        // NetworkFPCControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnHostActivated(const AZ::EntityId& entityId);
        void OnNonAutonomousClientActivated(const AZ::EntityId& entityId);

        // FirstPersonControllerRequestBus
        AZ::EntityId GetCharacterEntityId() const override;
        AZ::EntityId GetActiveCameraEntityId() const override;
        AZ::Entity* GetActiveCameraEntityPtr() const override;
        void SetCameraEntity(const AZ::EntityId cameraEntityId) override;
        bool GetMakeCameraChildOfCharacter() const override;
        void SetMakeCameraChildOfCharacter(const bool makeCameraChildOfCharacter) override;
        bool IsCameraChildOfCharacter() override;
        bool GetCameraSmoothFollow() const override;
        void SetCameraSmoothFollow(const bool cameraSmoothFollow) override;
        bool GetNetworkFPCKeepCameraAtCharacter() const override;
        void SetNetworkFPCKeepCameraAtCharacter(const bool networkFPCKeepCameraAtCharacter) override;
        void SetParentChangeDoNotUpdate(const AZ::EntityId& entityId) override;
        void SetParentChangeUpdate(const AZ::EntityId& entityId) override;
        AZ::OnParentChangedBehavior GetParentChangeBehavior(const AZ::EntityId& entityId) const override;
        float GetEyeHeight() const override;
        void SetEyeHeight(const float eyeHeight) override;
        float GetCameraLocalZTravelDistance() const override;
        AZ::TransformInterface* GetCameraRotationTransform() const override;
        void ReacquireChildEntityIds() override;
        AZStd::vector<AZ::EntityId> GetChildEntityIds() const override;
        void ReacquireCapsuleDimensions() override;
        void ReacquireMaxSlopeAngle() override;
        AZStd::string GetForwardEventName() const override;
        void SetForwardEventName(const AZStd::string& strForward) override;
        float GetForwardScale() const override;
        void SetForwardScale(const float forwardScale) override;
        float GetForwardInputValue() const override;
        void SetForwardInputValue(const float forwardValue) override;
        AZStd::string GetBackEventName() const override;
        void SetBackEventName(const AZStd::string& strBack) override;
        float GetBackScale() const override;
        void SetBackScale(const float backScale) override;
        float GetBackInputValue() const override;
        void SetBackInputValue(const float backValue) override;
        AZStd::string GetLeftEventName() const override;
        void SetLeftEventName(const AZStd::string& strLeft) override;
        float GetLeftScale() const override;
        void SetLeftScale(const float leftScale) override;
        float GetLeftInputValue() const override;
        void SetLeftInputValue(const float leftValue) override;
        AZStd::string GetRightEventName() const override;
        void SetRightEventName(const AZStd::string& strRight) override;
        float GetRightScale() const override;
        void SetRightScale(const float rightScale) override;
        float GetRightInputValue() const override;
        void SetRightInputValue(const float rightValue) override;
        AZStd::string GetYawEventName() const override;
        void SetYawEventName(const AZStd::string& strYaw) override;
        float GetYawInputValue() const override;
        void SetYawInputValue(const float yawValue) override;
        AZStd::string GetPitchEventName() const override;
        void SetPitchEventName(const AZStd::string& strPitch) override;
        float GetPitchInputValue() const override;
        void SetPitchInputValue(const float pitchValue) override;
        AZStd::string GetSprintEventName() const override;
        void SetSprintEventName(const AZStd::string& strSprint) override;
        float GetSprintInputValue() const override;
        void SetSprintInputValue(const float sprintValue) override;
        float GetSprintEffectiveValue() const override;
        void SetSprintEffectiveValue(const float sprintEffectiveValue) override;
        bool GetSprintInputEngaged() const override;
        void SetSprintInputEngaged(const bool sprintInputEngaged) override;
        AZStd::string GetCrouchEventName() const override;
        void SetCrouchEventName(const AZStd::string& strCrouch) override;
        float GetCrouchInputValue() const override;
        void SetCrouchInputValue(const float crouchValue) override;
        AZStd::string GetJumpEventName() const override;
        void SetJumpEventName(const AZStd::string& strJump) override;
        float GetJumpInputValue() const override;
        void SetJumpInputValue(const float jumpValue) override;
        bool GetForwardBackCancelOut() const override;
        void SetForwardBackCancelOut(const bool forwardBackCancelOut) override;
        bool GetGrounded() const override;
        float GetFellDistance() const override;
        float GetSoonFellDistance() const override;
        void SetGroundedForTick(const bool grounded) override;
        AZ::u16 GetNumTicksRecentGrounded() const override;
        void SetNumTicksRecentGrounded(const AZ::u16 numTicksRecentGrounded) override;
        bool GetScriptJump() const override;
        void SetScriptJump(const bool scriptJump) override;
        AZStd::vector<AZ::EntityId> GetGroundHitEntityIds() const override;
        AZStd::vector<AzPhysics::SceneQueryHit> GetGroundSceneQueryHits() const override;
        AZStd::vector<AzPhysics::SceneQueryHit> GetGroundCloseSceneQueryHits() const override;
        AZStd::vector<AzPhysics::SceneQueryHit> GetGroundCloseCoyoteTimeSceneQueryHits() const override;
        AZ::Vector3 GetGroundSumNormalsDirection() const override;
        AZ::Vector3 GetGroundCloseSumNormalsDirection() const override;
        AZStd::vector<AzPhysics::SceneQueryHit> GetCharacterHitsInGroupName(const AZStd::string& groupName) const override;
        AZStd::vector<AZ::EntityId> GetEntityIdsCharacterHitInGroupName(const AZStd::string& groupName) const override;
        bool GetGroundClose() const override;
        void SetGroundCloseForTick(const bool groundClose) override;
        AZStd::string GetGroundedCollisionGroupName() const override;
        void SetGroundedCollisionGroup(const AZStd::string& groundedCollisionGroup) override;
        float GetAirTime() const override;
        float GetGravity() const override;
        void SetGravity(const float gravity) override;
        AZ::Vector3 GetTargetVelocityWorld() const override;
        AZ::Vector3 GetTargetVelocityHeading() const override;
        float GetVelocityCloseTolerance() const override;
        void SetVelocityCloseTolerance(const float velocityCloseTolerance) override;
        AZ::Vector3 GetVelocityXCrossYDirection() const override;
        void SetVelocityXCrossYDirection(const AZ::Vector3& velocityXCrossYDirection) override;
        bool GetVelocityXCrossYTracksNormal() const override;
        void SetVelocityXCrossYTracksNormal(const bool velocityXCrossYTracksNormal) override;
        bool GetSpeedReducedWhenMovingUpInclines() const override;
        void SetSpeedReducedWhenMovingUpInclines(const bool movingUpInclineSlowed) override;
        AZ::Vector3 GetVelocityZPosDirection() const override;
        void SetVelocityZPosDirection(const AZ::Vector3& velocityZPosDirection) override;
        AZ::Vector3 GetSphereCastsAxisDirectionPose() const override;
        void SetSphereCastsAxisDirectionPose(const AZ::Vector3& sphereCastsAxisDirectionPose) override;
        float GetJumpHeldGravityFactor() const override;
        void SetJumpHeldGravityFactor(const float jumpHeldGravityFactor) override;
        float GetJumpFallingGravityFactor() const override;
        void SetJumpFallingGravityFactor(const float jumpFallingGravityFactor) override;
        float GetJumpAccelFactor() const override;
        void SetJumpAccelFactor(const float jumpAccelFactor) override;
        bool GetUpdateXYAscending() const override;
        void SetUpdateXYAscending(const bool updateXYAscending) override;
        bool GetUpdateXYDescending() const override;
        void SetUpdateXYDescending(const bool updateXYDescending) override;
        bool GetUpdateXYOnlyNearGround() const override;
        void SetUpdateXYOnlyNearGround(const bool updateXYOnlyNearGround) override;
        bool GetAddVelocityForTimestepVsTick() const override;
        void SetAddVelocityForTimestepVsTick(const bool addVelocityForTimestepVsTick) override;
        float GetPhysicsTimestepScaleFactor() const override;
        void SetPhysicsTimestepScaleFactor(const float physicsTimestepScaleFactor) override;
        bool GetScriptSetsTargetVelocityXY() const override;
        void SetScriptSetsTargetVelocityXY(const bool scriptSetsTargetVelocityXY) override;
        AZ::Vector2 GetScriptTargetVelocityXY() const override;
        void SetScriptTargetVelocityXY(const AZ::Vector2& scriptTargetVelocityXY) override;
        AZ::Vector2 GetScriptTargetVelocityXYWorld() const override;
        void SetScriptTargetVelocityXYWorld(const AZ::Vector2& scriptTargetVelocityXYWorld) override;
        float GetScriptTargetVelocityXYEulerAngle() const override;
        AZ::Vector2 GetCorrectedVelocityXY() const override;
        void SetCorrectedVelocityXY(const AZ::Vector2& correctedVelocityXY) override;
        float GetCorrectedVelocityZ() const override;
        void SetCorrectedVelocityZ(const float correctedVelocityZ) override;
        AZ::Vector2 GetApplyVelocityXY() const override;
        void SetApplyVelocityXY(const AZ::Vector2& applyVelocityXY) override;
        AZ::Vector2 GetNextLikelyApplyVelocityXY() const override;
        AZ::Vector3 GetAddVelocityWorld() const override;
        void SetAddVelocityWorld(const AZ::Vector3& addVelocityWorld) override;
        AZ::Vector3 GetAddVelocityHeading() const override;
        void SetAddVelocityHeading(const AZ::Vector3& addVelocityHeading) override;
        float GetApplyVelocityZ() const override;
        void SetApplyVelocityZ(const float applyVelocityZ) override;
        bool GetEnableImpulses() const override;
        void SetEnableImpulses(const bool enableImpulses) override;
        bool GetImpulseDecelUsesFriction() const override;
        void SetImpulseDecelUsesFriction(const bool impulseDecelUsesFriction) override;
        AZ::Vector3 GetLinearImpulse() const override;
        void SetLinearImpulse(const AZ::Vector3& linearImpulse) override;
        void ApplyLinearImpulse(const AZ::Vector3& addLinearImpulse) override;
        AZ::Vector3 GetInitVelocityFromImpulse() const override;
        void SetInitVelocityFromImpulse(const AZ::Vector3& initVelocityFromImpulse) override;
        AZ::Vector3 GetVelocityFromImpulse() const override;
        void SetVelocityFromImpulse(const AZ::Vector3& velocityFromImpulse) override;
        float GetImpulseLinearDamp() const override;
        void SetImpulseLinearDamp(const float impulseLinearDamp) override;
        float GetImpulseConstantDecel() const override;
        void SetImpulseConstantDecel(const float impulseConstantDecel) override;
        float GetImpulseTotalLerpTime() const override;
        void SetImpulseTotalLerpTime(const float impulseTotalLerpTime) override;
        float GetImpulseLerpTime() const override;
        void SetImpulseLerpTime(const float impulseLerpTime) override;
        float GetCharacterMass() const override;
        void SetCharacterMass(const float characterMass) override;
        bool GetEnableCharacterHits() const override;
        void SetEnableCharacterHits(const bool enableCharacterHits) override;
        float GetHitRadiusPercentageIncrease() const override;
        void SetHitRadiusPercentageIncrease(const float hitRadiusPercentageIncrease) override;
        float GetHitRadiusPercentageIncreaseWhileIdle() const override;
        void SetHitRadiusPercentageIncreaseWhileIdle(const float hitRadiusPercentageIncreaseWhileIdle) override;
        float GetHitHeightPercentageIncrease() const override;
        void SetHitHeightPercentageIncrease(const float hitHeightPercentageIncrease) override;
        float GetHitExtraProjectionPercentage() const override;
        void SetHitExtraProjectionPercentage(const float hitExtraProjectionPercentage) override;
        AZStd::string GetCharacterHitCollisionGroupName() const override;
        void SetCharacterHitCollisionGroupByName(const AZStd::string& characterHitCollisionGroupName) override;
        AzPhysics::SceneQuery::QueryType GetCharacterHitBy() const override;
        void SetCharacterHitBy(const AzPhysics::SceneQuery::QueryType& characterHitBy) override;
        AZStd::vector<AZ::EntityId> GetCharacterHitEntityIds() const override;
        AZStd::vector<AzPhysics::SceneQueryHit> GetCharacterSceneQueryHits() const override;
        float GetJumpInitialVelocity() const override;
        void SetJumpInitialVelocity(const float jumpInitialVelocity) override;
        float GetJumpSecondInitialVelocity() const override;
        void SetJumpSecondInitialVelocity(const float jumpInitialVelocity) override;
        bool GetJumpReqRepress() const override;
        void SetJumpReqRepress(const bool jumpReqRepress) override;
        bool GetJumpRepressHoldCausesJump() const override;
        void SetJumpRepressHoldCausesJump(const bool jumpRepressHoldCausesJump) override;
        bool GetJumpHeldKeepsJumping() const override;
        void SetJumpHeldKeepsJumping(const bool jumpHeldKeepsJumping) override;
        bool GetJumpHeld() const override;
        void SetJumpHeld(const bool jumpHeld) override;
        bool GetDoubleJump() const override;
        void SetDoubleJump(const bool doubleJumpEnabled) override;
        bool GetFinalJumpPerformed() const override;
        void SetFinalJumpPerformed(const bool onFinalJump) override;
        float GetGroundedOffset() const override;
        void SetGroundedOffset(const float groundedSphereCastOffset) override;
        float GetGroundedExtraOffsetMultiplayerDynamic() const override;
        void SetGroundedExtraOffsetMultiplayerDynamic(const float groundedExtraOffsetMultiplayerDynamic) override;
        float GetGroundCloseOffset() const override;
        void SetGroundCloseOffset(const float groundCloseSphereCastOffset) override;
        float GetGroundCloseCoyoteTimeOffset() const override;
        void SetGroundCloseCoyoteTimeOffset(const float groundCloseCoyoteTimeOffset) override;
        float GetJumpHoldDistance() const override;
        void SetJumpHoldDistance(const float jumpHoldDistance) override;
        float GetJumpHeadSphereCastOffset() const override;
        void SetJumpHeadSphereCastOffset(const float jumpHeadSphereCastOffset) override;
        bool GetHeadHitSetsApogee() const override;
        void SetHeadHitSetsApogee(const bool headHitSetsApogee) override;
        float GetFellFromHeight() const override;
        void SetFellFromHeight(const float fellFromHeight) override;
        bool GetHeadHit() const override;
        void SetHeadHit(const bool headHit) override;
        bool GetJumpHeadIgnoreDynamicRigidBodies() const override;
        void SetJumpHeadIgnoreDynamicRigidBodies(const bool jumpHeadIgnoreDynamicRigidBodies) override;
        AZStd::string GetHeadCollisionGroupName() const override;
        void SetHeadCollisionGroupByName(const AZStd::string& headCollisionGroupName) override;
        AZStd::vector<AZ::EntityId> GetHeadHitEntityIds() const override;
        bool GetStandPrevented() const override;
        void SetStandPrevented(const bool standPrevented) override;
        bool GetJumpWhileCrouched() const override;
        void SetJumpWhileCrouched(const bool jumpWhileCrouched) override;
        float GetCoyoteTime() const override;
        void SetCoyoteTime(const float coyoteTime) override;
        bool GetCoyoteTimeNoGravityActive() const override;
        void SetCoyoteTimeNoGravityActive(const bool coyoteTimeNoGravityActive) override;
        bool GetUngroundedDueToJump() const override;
        void SetUngroundedDueToJump(const bool ungroundedDueToJump) override;
        bool GetApplyGravityDuringCoyoteTime() const override;
        void SetApplyGravityDuringCoyoteTime(const bool applyGravityDuringCoyoteTime) override;
        bool GetCoyoteTimeTracksLastNormal() const override;
        void SetCoyoteTimeTracksLastNormal(const bool coyoteTimeTracksLastNormal) override;
        bool GetStandIgnoreDynamicRigidBodies() const override;
        void SetStandIgnoreDynamicRigidBodies(const bool standIgnoreDynamicRigidBodies) override;
        AZStd::string GetStandCollisionGroupName() const override;
        void SetStandCollisionGroupByName(const AZStd::string& standCollisionGroupName) override;
        AZStd::string GetCollisionGroupName(AzPhysics::CollisionGroup& collisionGroup) const override;
        AZStd::vector<AZ::EntityId> GetStandPreventedEntityIds() const override;
        float GetGroundSphereCastsRadiusPercentageIncrease() const override;
        void SetGroundSphereCastsRadiusPercentageIncrease(const float groundSphereCastsRadiusPercentageIncrease) override;
        float GetGroundCloseCoyoteTimeRadiusPercentageIncrease() const override;
        void SetGroundCloseCoyoteTimeRadiusPercentageIncrease(const float groundCloseCoyoteTimeRadiusPercentageIncrease) override;
        float GetMaxGroundedAngleDegrees() const override;
        void SetMaxGroundedAngleDegrees(const float maxGroundedAngleDegrees) override;
        float GetTopWalkSpeed() const override;
        void SetTopWalkSpeed(const float speed) override;
        float GetWalkAcceleration() const override;
        void SetWalkAcceleration(const float accel) override;
        float GetTotalLerpTime() const override;
        void SetTotalLerpTime(const float totalLerpTime) override;
        float GetLerpTime() const override;
        void SetLerpTime(const float lerpTime) override;
        float GetDecelerationFactor() const override;
        void SetDecelerationFactor(const float decel) override;
        float GetOpposingDecel() const override;
        void SetOpposingDecel(const float opposingDecel) override;
        bool GetAccelerating() const override;
        bool GetDecelerationFactorApplied() const override;
        bool GetOpposingDecelFactorApplied() const override;
        bool GetInstantVelocityRotation() const override;
        void SetInstantVelocityRotation(const bool instantVelocityRotation) override;
        bool GetVelocityXYIgnoresObstacles() const override;
        void SetVelocityXYIgnoresObstacles(const bool velocityXYIgnoresObstacles) override;
        bool GetGravityIgnoresObstacles() const override;
        void SetGravityIgnoresObstacles(const bool gravityIgnoresObstacles) override;
        bool GetPosZIgnoresObstacles() const override;
        void SetPosZIgnoresObstacles(const bool posZIgnoresObstacles) override;
        bool GetJumpAllowedWhenGravityPrevented() const override;
        void SetJumpAllowedWhenGravityPrevented(const bool jumpAllowedWhenGravityPrevented) override;
        bool GetVelocityXYObstructed() const override;
        void SetVelocityXYObstructed(const bool velocityXYObstructed) override;
        bool GetGravityPrevented() const override;
        void SetGravityPrevented(const bool gravityPrevented) override;
        float GetSprintScaleForward() const override;
        void SetSprintScaleForward(const float sprintScaleForward) override;
        float GetSprintScaleBack() const override;
        void SetSprintScaleBack(const float sprintScaleBack) override;
        float GetSprintScaleLeft() const override;
        void SetSprintScaleLeft(const float sprintScaleLeft) override;
        float GetSprintScaleRight() const override;
        void SetSprintScaleRight(const float sprintScaleRight) override;
        float GetSprintAccelScale() const override;
        void SetSprintAccelScale(const float sprintAccelScale) override;
        float GetSprintAccumulatedAccel() const override;
        void SetSprintAccumulatedAccel(const float sprintAccumulatedAccel) override;
        float GetSprintMaxTime() const override;
        void SetSprintMaxTime(const float sprintMaxTime) override;
        float GetSprintHeldTime() const override;
        void SetSprintHeldTime(const float sprintHeldDuration) override;
        float GetSprintRegenRate() const override;
        void SetSprintRegenRate(const float sprintRegenRate) override;
        float GetStaminaPercentage() const override;
        void SetStaminaPercentage(const float staminaPercentage) override;
        bool GetStaminaIncreasing() const override;
        bool GetStaminaDecreasing() const override;
        bool GetSprintUsesStamina() const override;
        void SetSprintUsesStamina(const bool sprintUsesStamina) override;
        bool GetRegenerateStaminaAutomatically() const override;
        void SetRegenerateStaminaAutomatically(const bool regenerateStaminaAutomatically) override;
        bool GetSprinting() const override;
        float GetSprintTotalCooldownTime() const override;
        void SetSprintTotalCooldownTime(const float sprintCooldownTimerTime) override;
        float GetSprintCooldownTimer() const override;
        void SetSprintCooldownTimer(const float sprintCooldownTimer) override;
        float GetSprintPauseTime() const override;
        void SetSprintPauseTime(const float sprintPauseTime) override;
        float GetSprintPause() const override;
        void SetSprintPause(const float sprintPause) override;
        bool GetSprintBackwards() const override;
        void SetSprintBackwards(const bool sprintBackwards) override;
        bool GetSprintWhileCrouched() const override;
        void SetSprintWhileCrouched(const bool sprintWhileCrouched) override;
        bool GetSprintInAir() const override;
        void SetSprintInAir(const bool sprintInAir) override;
        bool GetSprintEnableToggle() const override;
        void SetSprintEnableToggle(const bool sprintEnableToggle) override;
        bool GetSprintToggleAutomatically() const override;
        void SetSprintToggleAutomatically(const bool sprintToggleAutomatically) override;
        bool GetSprintAutoToggleOutOfCrouch() const override;
        void SetSprintAutoToggleOutOfCrouch(const bool sprintAutoToggleOutOfCrouch) override;
        bool GetCrouching() const override;
        void SetCrouching(const bool crouching) override;
        bool GetCrouched() const override;
        bool GetStanding() const override;
        float GetCrouchedPercentage() const override;
        bool GetCrouchScriptLocked() const override;
        void SetCrouchScriptLocked(const bool crouchScriptLocked) override;
        float GetCrouchScale() const override;
        void SetCrouchScale(const float crouchScale) override;
        float GetCrouchDistance() const override;
        void SetCrouchDistance(const float crouchDistance) override;
        bool GetCrouchingDownMove() const override;
        bool GetStandingUpMove() const override;
        float GetUncrouchHeadSphereCastOffset() const override;
        void SetUncrouchHeadSphereCastOffset(const float uncrouchHeadSphereCastOffset) override;
        bool GetCrouchEnableToggle() const override;
        void SetCrouchEnableToggle(const bool crouchEnableToggle) override;
        bool GetCrouchJumpCausesStanding() const override;
        void SetCrouchJumpCausesStanding(const bool crouchJumpCausesStanding) override;
        bool GetCrouchPendJumps() const override;
        void SetCrouchPendJumps(const bool crouchPendJumps) override;
        bool GetCrouchSprintCausesStanding() const override;
        void SetCrouchSprintCausesStanding(const bool crouchSprintCausesStanding) override;
        bool GetCrouchPriorityWhenSprintPressed() const override;
        void SetCrouchPriorityWhenSprintPressed(const bool crouchPriorityWhenSprintPressed) override;
        bool GetCrouchWhenNotGrounded() const override;
        void SetCrouchWhenNotGrounded(const bool crouchWhenNotGrounded) override;
        float GetCrouchDownProportionalGain() const override;
        void SetCrouchDownProportionalGain(const float crouchDownProportionalGain) override;
        float GetCrouchDownIntegralGain() const override;
        void SetCrouchDownIntegralGain(const float crouchDownIntegralGain) override;
        float GetCrouchDownDerivativeGain() const override;
        void SetCrouchDownDerivativeGain(const float crouchDownDerivativeGain) override;
        float GetCrouchDownIntegralWindupLimit() const override;
        void SetCrouchDownIntegralWindupLimit(const float crouchDownIntegralWindupLimit) override;
        float GetCrouchDownDerivativeFilterAlpha() const override;
        void SetCrouchDownDerivativeFilterAlpha(const float crouchDownDerivativeFilterAlpha) override;
        float GetStandUpProportionalGain() const override;
        void SetStandUpProportionalGain(const float standUpProportionalGain) override;
        float GetStandUpIntegralGain() const override;
        void SetStandUpIntegralGain(const float standUpIntegralGain) override;
        float GetStandUpDerivativeGain() const override;
        void SetStandUpDerivativeGain(const float standUpDerivativeGain) override;
        float GetStandUpIntegralWindupLimit() const override;
        void SetStandUpIntegralWindupLimit(const float standUpIntegralWindupLimit) override;
        float GetStandUpDerivativeFilterAlpha() const override;
        void SetStandUpDerivativeFilterAlpha(const float standUpDerivativeFilterAlpha) override;
        PidController<float>::DerivativeCalculationMode GetCrouchDownDerivativeMode() const override;
        void SetCrouchDownDerivativeMode(const PidController<float>::DerivativeCalculationMode& crouchDownDerivativeMode) override;
        PidController<float>::DerivativeCalculationMode GetStandUpDerivativeMode() const override;
        void SetStandUpDerivativeMode(const PidController<float>::DerivativeCalculationMode& standUpDerivativeMode) override;
        bool GetSprintViaScript() const override;
        void SetSprintViaScript(const bool sprintViaScript) override;
        bool GetSprintEnableDisable() const override;
        void SetSprintEnableDisable(const bool sprintEnableDisable) override;
        bool GetEnableCameraCharacterRotation() const override;
        void SetEnableCameraCharacterRotation(const bool enableCameraCharacterRotation) override;
        float GetCharacterAndCameraYawSensitivity() const override;
        void SetCharacterAndCameraYawSensitivity(const float yawSensitivity) override;
        float GetCameraPitchSensitivity() const override;
        void SetCameraPitchSensitivity(const float pitchSensitivity) override;
        float GetCameraPitchMaxAngleRadians() const override;
        void SetCameraPitchMaxAngleRadians(const float pitchMaxAngleRadians) override;
        float GetCameraPitchMaxAngleDegrees() const override;
        void SetCameraPitchMaxAngleDegrees(const float pitchMaxAngleDegrees) override;
        float GetCameraPitchMinAngleRadians() const override;
        void SetCameraPitchMinAngleRadians(const float pitchMinAngleRadians) override;
        float GetCameraPitchMinAngleDegrees() const override;
        void SetCameraPitchMinAngleDegrees(const float pitchMinAngleDegrees) override;
        float GetCameraRotationDampFactor() const override;
        void SetCameraRotationDampFactor(const float rotationDamp) override;
        AZ::TransformInterface* GetCharacterTransformInterfacePtr() const override;
        AZ::Transform GetCharacterTransform() const override;
        void SetCharacterTransform(const AZ::Transform& characterTransform) override;
        AZ::Vector3 GetCharacterWorldTranslation() const override;
        void SetCharacterWorldTranslation(const AZ::Vector3& characterWorldTranslation) override;
        void UpdateCharacterAndCameraYaw(
            const float characterAndCameraYawAngle, const bool updateCharacterAndCameraYawConsidersInput = true) override;
        void UpdateCameraPitch(const float cameraPitchAngle, const bool updateCameraPitchConsidersInput = true) override;
        float GetHeading() const override;
        void SetHeadingForTick(const float currentHeading) override;
        float GetPitch() const override;
        float GetYaw() const override;
        bool GetIsAutonomousClient() const override;
        void SetIsAutonomousClient(const bool isAutonomousClient) override;
        bool GetIsServer() const override;
        void SetIsServer(const bool isServer) override;
        bool GetIsHost() const override;
        void SetIsHost(const bool isHost) override;
        bool GetIsNetBot() const override;
        void SetIsNetBot(const bool isNetBot) override;
        AZStd::vector<AZ::EntityId> GetOtherPlayerEntityIds() const override;
        AZStd::vector<AZ::EntityId> GetNetBotEntityIds() const override;
#ifdef NETWORKFPC
        AZStd::string GetStringNetEntityIdById(const AZ::EntityId& entityId) const override;
        Multiplayer::NetEntityId GetNetEntityIdById(const AZ::EntityId& entityId) const override;
        AZ::EntityId GetEntityIdByNetId(const Multiplayer::NetEntityId& netEntityId) const override;
        AZ::EntityId GetEntityIdByStringNetId(const AZStd::string& intNetEntityId) const override;
#endif
        static AZ::Vector2 CreateEllipseScaledVector(
            const AZ::Vector2& unscaledVector, float forwardScale, float backScale, float leftScale, float rightScale);
        static AZ::Vector3 TiltVectorXCrossY(const AZ::Vector2& vXY, const AZ::Vector3& newXCrossYDirection);
        static AZ::Vector3 GetVectorAnglesBetweenVectorsRadians(const AZ::Vector3& v1, const AZ::Vector3& v2);
        static AZ::Vector3 GetVectorAnglesBetweenVectorsDegrees(const AZ::Vector3& v1, const AZ::Vector3& v2);
        static float SlerpHeadings(const float a, const float b, const float t);
        static float ConvertVectorToHeading(const AZ::Vector3& convertVector);
        static AzPhysics::SimulatedBodyHandle GetSceneQueryHitSimulatedBodyHandle(const AzPhysics::SceneQueryHit& hit);
        static bool GetSceneQueryHitIsInGroupName(const AzPhysics::SceneQueryHit& hit, const AZStd::string& groupName);
        static bool GetLayerNameIsInGroupName(const AZStd::string& layerName, const AZStd::string& groupName);
        static AZStd::string GetSceneQueryHitLayerName(const AzPhysics::SceneQueryHit& hit);
        static AzPhysics::SceneQuery::ResultFlags GetSceneQueryHitResultFlags(const AzPhysics::SceneQueryHit& hit);
        static AZ::EntityId GetSceneQueryHitEntityId(const AzPhysics::SceneQueryHit& hit);
        static AZ::Vector3 GetSceneQueryHitNormal(const AzPhysics::SceneQueryHit& hit);
        static AZ::Vector3 GetSceneQueryHitPosition(const AzPhysics::SceneQueryHit& hit);
        static float GetSceneQueryHitDistance(const AzPhysics::SceneQueryHit& hit);
        static Physics::MaterialId GetSceneQueryHitMaterialId(const AzPhysics::SceneQueryHit& hit);
        static AZStd::shared_ptr<Physics::Material> GetSceneQueryHitMaterialPtr(const AzPhysics::SceneQueryHit& hit);
        static AZ::Data::Asset<Physics::MaterialAsset> GetSceneQueryHitMaterialAsset(const AzPhysics::SceneQueryHit& hit);
        static AZ::Data::AssetId GetSceneQueryHitMaterialAssetId(const AzPhysics::SceneQueryHit& hit);
        static float GetSceneQueryHitDynamicFriction(const AzPhysics::SceneQueryHit& hit);
        static float GetSceneQueryHitStaticFriction(const AzPhysics::SceneQueryHit& hit);
        static float GetSceneQueryHitRestitution(const AzPhysics::SceneQueryHit& hit);
        static Physics::Shape* GetSceneQueryHitShapePtr(const AzPhysics::SceneQueryHit& hit);
        static AZStd::vector<AZ::EntityId> GetPlayerEntityIdsOnServer();
        static AZStd::vector<AZStd::string> GetPlayerStringNetEntityIdsOnServer();
        static AZStd::vector<AZ::EntityId> GetNetBotEntityIdsOnServer();
        static AZStd::vector<AZStd::string> GetBotStringNetEntityIdsOnServer();
        static AZStd::vector<AZStd::string> GetPlayerNetEntityIdStrings();
        static AZStd::vector<AZStd::string> GetBotNetEntityIdStrings();
        static AZ::EntityId GetAutonomousClientEntityId();
        static AZ::EntityId GetHostEntityId();
        bool GetNetworkFPCAllowAllMovementInputs() const override;
        void SetNetworkFPCAllowAllMovementInputs(const bool allowAllMovementInputs) override;
        bool GetNetworkFPCAllowRotationInputs() const override;
        void SetNetworkFPCAllowRotationInputs(const bool allowRotationInputs) override;
        AZ::TimeMs GetNetworkFPCHostTimeMs() const override;
        bool GetLocallyEnableNetworkFPC() const override;
        void SetLocallyEnableNetworkFPC(const bool networkFPCEnabled) override;
        bool GetIsNetworkingActive() const override;
        void IgnoreInputs(const bool ignoreInputs) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

        inline static AZStd::vector<AZ::EntityId> m_playerEntityIdsOnServer;
        inline static AZStd::vector<AZStd::string> m_playerStringNetEntityIds;
        inline static AZStd::vector<AZ::EntityId> m_netBotEntityIdsOnServer;
        inline static AZStd::vector<AZStd::string> m_botStringNetEntityIds;
        inline static bool m_reacquirePlayerBotStringNetEntityIds = true;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // Assigns a camera to m_cameraEntityId if none is specified
        void OnCameraAdded(const AZ::EntityId& cameraId);

        // Set the first-person camera position
        void InitializeCameraTranslation();

        // NetworkFPC object
#ifdef NETWORKFPC
        NetworkFPC* m_networkFPCObject = nullptr;
        NetworkFPCController* m_networkFPCControllerObject = nullptr;
        NetworkFPCBotAnimation* m_networkFPCBotAnimationObject = nullptr;
        NetworkFPCBotAnimationController* m_networkFPCBotAnimationControllerObject = nullptr;
#else
        bool* m_networkFPCObject = nullptr;
        bool* m_networkFPCControllerObject = nullptr;
        bool* m_networkFPCBotAnimationObject = nullptr;
        bool* m_networkFPCBotAnimationControllerObject = nullptr;
#endif

        // Active camera entity pointer and ID
        AZ::Entity* m_activeCameraEntity = nullptr;
        AZ::EntityId m_cameraEntityId;

        // Whether to force the camera to be a child of the character entity
        bool m_makeCameraChildOfCharacter = false;

        // Child EntityIds
        bool m_obtainedChildIds = false;
        AZStd::vector<AZ::EntityId> m_children;

        // Called on each tick
        void ProcessInput(const float deltaTime, const AZ::u8 tickTimestepNetwork);

        // Various methods used to implement the First Person Controller functionality
        void CheckGrounded(const float deltaTime);
        void UpdateVelocityXY(const float deltaTime);
        void AcquireSumOfGroundNormals();
        void UpdateJumpMaxHoldTime();
        void UpdateVelocityZ(const float deltaTime);
        void UpdateRotation(const float deltaTime, const AZ::u8 tickTimestepNetwork);
        AZ::Vector2 LerpVelocityXY(const AZ::Vector2& targetVelocity, const float deltaTime);
        void ApplyMovingUpInclineXYSpeedFactor();
        void LerpCameraToCharacter(const float deltaTime);
        void SmoothRotation();
        void ResetCameraToCharacter();
        void CaptureCharacterEyeTranslation();
        void SprintManager(const AZ::Vector2& targetVelocity, const float deltaTime);
        void CrouchManager(const float deltaTime);
        void CheckCharacterMovementObstructed();
        void ProcessLinearImpulse(const float deltaTime);
        void ProcessCharacterHits(const float deltaTime);
        void GetNetworkFPCProperties();
        void SetNetworkFPCProperties() const;

        // Method for getting a pointer to an entity
        AZ::Entity* GetEntityPtr(const AZ::EntityId& entityId) const;

        // These getter methods are not exposed to the request bus, they're used for the visibility attribute in the editor
        bool GetCameraNotSmoothFollow() const;
        bool GetEnableImpulsesAndNotDecelUsesFriction() const;
        bool GetCoyoteTimeGreaterThanZero() const;
        bool GetCoyoteTimeGreaterThanZeroAndNoGravityDuring() const;

        // Stores the previous frame tick deltaTime, previous physics timestep, and previous NetworkFPC tick deltaTime
        float m_prevTimestep = 1.f / 60.f;
        float m_prevNetworkFPCDeltaTime = 0.033f;

        // Provides the functionality when AddVelocityForPhysicsTimestep is used
        void OnSceneSimulationStart(float physicsTimestep);
        void OnSceneSimulationFinish([[maybe_unused]] float physicsTimestep);
        AzPhysics::SceneEvents::OnSceneSimulationStartHandler m_sceneSimulationStartHandler;
        AzPhysics::SceneEvents::OnSceneSimulationFinishHandler m_sceneSimulationFinishHandler;
        AzPhysics::SceneHandle m_attachedSceneHandle = AzPhysics::InvalidSceneHandle;
        bool m_addVelocityForTimestepVsTick = true;
        bool m_cameraSmoothFollow = true;
        bool m_cameraTranslationOverwritten = false;
        bool m_cameraLocalZOverwritten = false;
        bool m_cameraRotationOverwritten = false;
        float m_physicsTimestepScaleFactor = 1.f;

        // Camera interpolation variables
        float m_eyeHeight = 1.6f;
        float m_physicsTimeAccumulator = 0.f;
        AZ::Vector3 m_prevCharacterEyeTranslation = AZ::Vector3::CreateZero();
        AZ::Vector3 m_currentCharacterEyeTranslation = AZ::Vector3::CreateZero();

        // Velocity application variables
        AZ::Vector2 m_applyVelocityXY = AZ::Vector2::CreateZero();
        AZ::Vector2 m_nextLikelyApplyVelocityXY = AZ::Vector2::CreateZero();
        AZ::Vector3 m_targetVelocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_currentPhysicsVelocity = AZ::Vector3::CreateZero();
        AZ::Vector3 m_sampledVelocity = AZ::Vector3::CreateZero();
        AZ::Vector2 m_scriptTargetVelocityXY = AZ::Vector2::CreateZero();
        AZ::Vector3 m_addVelocityWorld = AZ::Vector3::CreateZero();
        AZ::Vector3 m_addVelocityHeading = AZ::Vector3::CreateZero();
        AZ::Vector2 m_targetVelocityXY = AZ::Vector2::CreateZero();
        AZ::Vector2 m_prevApplyVelocityXY = AZ::Vector2::CreateZero();
        AZ::Vector2 m_correctedVelocityXY = AZ::Vector2::CreateZero();
        float m_velocityCloseTolerance = 1.f;
        float m_velocityCloseToleranceGravity = 1.f;
        bool m_instantVelocityRotation = false;
        bool m_velocityXYIgnoresObstacles = true;
        bool m_gravityIgnoresObstacles = false;
        bool m_posZIgnoresObstacles = true;
        bool m_jumpAllowedWhenGravityPrevented = true;
        bool m_velocityXYObstructed = false;
        bool m_gravityPrevented[2] = { false, false };

        // Determines whether the character's X&Y target velocity
        // will be set the request bus (script), in effect the entire time this variable is true
        bool m_scriptSetsTargetVelocityXY = false;

        // Angles used to rotate the camera
        AZ::Vector3 m_cameraRotationAnglesDelta = AZ::Vector3::CreateZero();

        // Top walk speed
        float m_speed = 5.f;

        // Used to track where we are along lerping the velocity between the two values
        float m_lerpTime = 0.f;
        float m_totalLerpTime = 0.f;

        // Sprint application variables
        float m_sprintEffectiveValue = 0.f;
        float m_sprintAccelValue = 1.f;
        float m_sprintPrevValue = 0.f;
        float m_sprintVelocityAdjust = 0.f;
        float m_sprintScaleForward = 1.5f;
        float m_sprintScaleBack = 1.f;
        float m_sprintScaleLeft = 1.25f;
        float m_sprintScaleRight = 1.25f;
        float m_sprintAccelAdjust = 0.f;
        bool m_sprintStopAccelAdjustCaptured = false;
        float m_sprintAccumulatedAccel = 0.f;
        float m_sprintHeldDuration = 0.f;
        float m_sprintRegenRate = 1.f;
        float m_sprintMaxTime = 120.f;
        float m_sprintCooldownTimer = 0.f;
        float m_sprintTotalCooldownTime = 1.f;
        float m_sprintPauseTime = (m_sprintTotalCooldownTime > m_sprintMaxTime) ? 0.f : 0.1f * m_sprintTotalCooldownTime;
        float m_sprintPause = 0.f;
        bool m_sprintBackwards = true;
        bool m_sprintWhileCrouched = false;
        bool m_sprintInAir = false;
        bool m_sprintEnableToggle = false;
        bool m_sprintInputEngaged = false;
        bool m_sprintToggleAutomatically = false;
        bool m_sprintAutoToggleOutOfCrouch = false;
        bool m_sprintAutoToggleQueued = false;
        bool m_sprintViaScript = false;
        bool m_sprintEnableDisable = false;
        bool m_staminaIncreasing = false;
        bool m_staminaDecreasing = false;
        bool m_sprintUsesStamina = true;
        bool m_regenerateStaminaAutomatically = true;

        // Stamina application variables
        float m_staminaPercentage =
            (m_sprintCooldownTimer == 0.f) ? 100.f * (m_sprintMaxTime - m_sprintHeldDuration) / m_sprintMaxTime : 0.f;

        // Crouch application variables
        float m_crouchDistance = 0.5f;
        float m_crouchPrevValue = 0.f;
        bool m_crouching = false;
        bool m_crouched = false;
        bool m_standing = true;
        bool m_crouchingDownMove = false;
        bool m_standingUpMove = false;
        float m_cameraLocalZTravelDistance = 0.f;
        bool m_crouchEnableToggle = true;
        bool m_crouchJumpCausesStanding = true;
        bool m_crouchPendJumps = true;
        bool m_crouchJumpPending = false;
        bool m_crouchSprintCausesStanding = true;
        bool m_crouchPriorityWhenSprintPressed = true;
        bool m_crouchScriptLocked = false;
        bool m_crouchWhenNotGrounded = false;
        float m_currentCrouchVelocity = 0.f;
        float m_crouchDownSettleTimer = 0.f;
        float m_standUpSettleTimer = 0.f;

        // Crouch Down PID Parameters
        float m_crouchDownProportionalGain = 200.f;
        float m_crouchDownIntegralGain = 0.f;
        float m_crouchDownDerivativeGain = 18.f;
        float m_crouchDownIntegralWindupLimit = 100.f;
        float m_crouchDownDerivativeFilterAlpha = 0.8f;
        PidController<float> m_crouchDownPidController;
        PidController<float>::DerivativeCalculationMode m_crouchDownDerivativeMode = PidController<float>::Velocity;

        // Stand Up PID Parameters
        float m_standUpProportionalGain = 200.f;
        float m_standUpIntegralGain = 0.f;
        float m_standUpDerivativeGain = 18.f;
        float m_standUpIntegralWindupLimit = 100.f;
        float m_standUpDerivativeFilterAlpha = 0.8f;
        PidController<float> m_standUpPidController;
        PidController<float>::DerivativeCalculationMode m_standUpDerivativeMode = PidController<float>::Velocity;

        // Jumping and gravity
        float m_gravity = -30.f;
        bool m_grounded = true;
        AZ::u16 m_numTicksRecentGrounded = 3;
        AZStd::vector<bool> m_prevNTicksGrounded = { true };
        AZ::Vector3 m_velocityXCrossYDirection = AZ::Vector3::CreateAxisZ();
        AZ::Vector3 m_prevVelocityXCrossYDirection = AZ::Vector3::CreateAxisZ();
        AZ::Vector3 m_coyoteVelocityXCrossYDirection = AZ::Vector3::CreateAxisZ();
        float m_movingUpInclineFactor = 1.f;
        bool m_jumpInclineVelocityXYCaptured = false;
        AZ::Vector3 m_groundCloseSumNormals = AZ::Vector3::CreateAxisZ();
        AZ::Vector3 m_velocityZPosDirection = AZ::Vector3::CreateAxisZ();
        bool m_velocityXCrossYTracksNormal = true;
        bool m_movingUpInclineSlowed = true;
        AZ::Vector3 m_sphereCastsAxisDirectionPose = AZ::Vector3::CreateAxisZ();
        AzPhysics::CollisionGroups::Id m_groundedCollisionGroupId = AzPhysics::CollisionGroups::Id();
        AzPhysics::CollisionGroup m_groundedCollisionGroup = AzPhysics::CollisionGroup::All;
        AZStd::vector<AzPhysics::SceneQueryHit> m_groundHits;
        AZStd::vector<AzPhysics::SceneQueryHit> m_groundCloseHits;
        AZStd::vector<AzPhysics::SceneQueryHit> m_groundCloseCoyoteTimeHits;
        float m_maxGroundedAngleDegrees = 30.f;
        bool m_scriptGrounded = true;
        bool m_scriptJump = false;
        bool m_scriptSetGroundTick = false;
        bool m_groundClose = true;
        bool m_groundCloseCoyoteTime = true;
        bool m_scriptGroundClose = true;
        bool m_scriptSetGroundCloseTick = false;
        float m_airTime = 0.f;
        float m_jumpInitialVelocity = 6.f;
        float m_jumpSecondInitialVelocity = 6.f;
        bool m_jumpHeld = false;
        bool m_jumpReqRepress = true;
        bool m_jumpRepressHoldCausesJump = true;
        bool m_jumpHeldKeepsJumping = false;
        float m_applyVelocityZ = 0.f;
        float m_applyVelocityZCurrentDelta = 0.f;
        float m_correctedVelocityZ = 0.f;
        float m_capsuleRadius = 0.3f;
        float m_capsuleHeight = 1.8f;
        float m_capsuleCurrentHeight = 1.8f;
        // The grounded sphere cast offset determines how far below the character's feet the ground is detected
        float m_groundedSphereCastOffset = 0.001f;
        // The grounded extra offset multiplayer dynamic is an extra projection distance for detecting dynamic bodies in multiplayer
        float m_groundedExtraOffsetMultiplayerDynamic = 0.8f;
        // The ground close sphere cast offset determines how far below the character's feet the ground is considered to be close
        float m_groundCloseSphereCastOffset = 0.5f;
        // The ground close coyote time offset determines how far below the character's feet the ground is considered to be close for Coyote
        // Time application
        float m_groundCloseCoyoteTimeOffset = 0.4f;
        // The sphere cast jump hold offset is used to determine initial (ascending) distance of the of the jump
        // where the m_jumpHeldGravityFactor is applied to the gravity
        float m_jumpHoldDistance = 0.8f;
        // The value of 41.5% was determined to work well based on testing
        float m_groundSphereCastsRadiusPercentageIncrease = 41.5;
        // If this is set to -100 then a raycast will be used instead of a spherecast
        float m_groundCloseCoyoteTimeRadiusPercentageIncrease = 20.f;
        float m_jumpHeldGravityFactor = 0.1f;
        // The m_jumpMaxHoldTime is computed inside UpdateJumpMaxHoldTime()
        float m_jumpMaxHoldTime = m_jumpHoldDistance /
            ((m_jumpInitialVelocity +
              sqrt(m_jumpInitialVelocity * m_jumpInitialVelocity + 2.f * m_gravity * m_jumpHeldGravityFactor * m_jumpHoldDistance)) /
             2.f);
        float m_jumpTimer = 0.f;
        float m_jumpFallingGravityFactor = 0.9f;
        bool m_doubleJumpEnabled = false;
        bool m_onFinalJump = false;
        float m_coyoteTime = 0.f;
        bool m_coyoteTimeNoGravityActive = false;
        bool m_coyoteTimeTracksLastNormal = true;
        bool m_ungroundedDueToJump = false;
        bool m_jumpCoyoteGravityPending = false;
        bool m_applyGravityDuringCoyoteTime = false;
        bool m_jumpHeadIgnoreDynamicRigidBodies = true;
        bool m_jumpWhileCrouched = false;
        bool m_headHit = false;
        bool m_headHitSetsApogee = true;
        float m_fellFromHeight = 0.f;
        float m_fellDistance = 0.f;
        float m_soonFellDistance = 0.f;
        AzPhysics::CollisionGroups::Id m_headCollisionGroupId = AzPhysics::CollisionGroups::Id();
        AzPhysics::CollisionGroup m_headCollisionGroup = AzPhysics::CollisionGroup::All;
        AZStd::vector<AZ::EntityId> m_characterHitEntityIds;
        AZStd::vector<AZ::EntityId> m_groundHitEntityIds;
        AZStd::vector<AZ::EntityId> m_headHitEntityIds;
        float m_jumpHeadSphereCastOffset = 0.2f;
        bool m_onFirstJump = false;
        bool m_onGroundSoonHit = false;

        // Variables used for impulses and hit detection
        bool m_enableImpulses = true;
        bool m_impulseDecelUsesFriction = true;
        AZ::Vector3 m_linearImpulse = AZ::Vector3::CreateZero();
        AZ::Vector3 m_initVelocityFromImpulse = AZ::Vector3::CreateZero();
        AZ::Vector3 m_velocityFromImpulse = AZ::Vector3::CreateZero();
        AZ::Vector2 m_applyVelocityXYFromImpulse = AZ::Vector2::CreateZero();
        float m_impulseConstantDecel = 10.f;
        float m_impulsePrevConstantDecel = 10.f;
        float m_impulseLinearDamp = 4.f;
        float m_impulseTotalLerpTime = 0.f;
        float m_impulseLerpTime = 0.f;
        float m_characterMass = 80.f;
        bool m_enableCharacterHits = true;
        float m_hitRadiusPercentageIncrease = 63.f;
        float m_hitRadiusPercentageIncreaseWhileIdle = 55.f;
        float m_hitHeightPercentageIncrease = 5.f;
        float m_hitExtraProjectionPercentage = 10.f;
        AzPhysics::CollisionGroups::Id m_characterHitCollisionGroupId = AzPhysics::CollisionGroups::Id();
        AzPhysics::CollisionGroup m_characterHitCollisionGroup = AzPhysics::CollisionGroup::All;
        AzPhysics::SceneQuery::QueryType m_characterHitBy = AzPhysics::SceneQuery::QueryType::StaticAndDynamic;
        AZStd::vector<AzPhysics::SceneQueryHit> m_characterHits;

        // Networking related variables (Note: m_isNetBot is true by default because it is set to false by NetworkFPC when autonomous)
        bool m_networkFPCEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
        AZStd::vector<AZ::EntityId> m_otherPlayerEntityIds;
        AZStd::vector<AZ::EntityId> m_netBotEntityIds;
#ifdef NETWORKFPC
        bool m_isNetBot = true;
#else
        bool m_isNetBot = false;
#endif
        bool m_networkFPCCameraAligned = false;
        // To have the camera not follow the character with multiplayer, m_cameraSmoothFollow will have to be set false as well
        bool m_networkFPCKeepCameraAtCharacter = true;
        float m_networkFPCRotationSliceAccumulator = 0.f;
        float m_networkFPCYawOvershootAngle = 0.f;

        // Variables used to determine when the X&Y velocity should be updated
        bool m_updateXYAscending = true;
        bool m_updateXYDescending = true;
        bool m_updateXYOnlyNearGround = false;

        // These default values work well, depending on OS mouse settings,
        // assuming the event value multiplier is 1.0
        float m_pitchSensitivity = 0.0035f;
        float m_yawSensitivity = 0.0035f;

        // Rotation-related variables
        bool m_enableCameraCharacterRotation = true;
        float m_currentHeading = 0.f;
        bool m_scriptSetCurrentHeadingTick = false;
        float m_currentPitch = 0.f;
        float m_prevCharacterPitch = 0.f;
        float m_prevCharacterRoll = 0.f;
        float m_cameraPitch = 0.f;
        float m_cameraRoll = 0.f;
        float m_cameraYaw = 0.f;
        AZ::Quaternion m_newLookRotationDelta = AZ::Quaternion::CreateZero();
        AZ::Vector3 m_newLookRotationDeltaRadians = AZ::Vector3::CreateZero();
        float m_rotationDamp = 30.f;
        float m_cameraPitchMaxAngle = AZ::Constants::HalfPi;
        float m_cameraPitchMinAngle = -AZ::Constants::HalfPi;
        AZ::TransformInterface* m_cameraRotationTransform = nullptr;

        // Used when a script wants to update the camera angle via the Request Bus or a gamepad
        bool m_rotatingPitchViaScriptGamepad = false;
        bool m_rotatingYawViaScriptGamepad = false;

        // Acceleration lerp movement
        float m_accel = 30.f;
        float m_jumpAccelFactor = 0.25f;
        bool m_accelerating = false;

        // Deceleration factor
        float m_decel = 1.5f;
        float m_opposingDecel = 2.f;
        float m_decelerationFactor = m_decel;
        bool m_decelerationFactorApplied = false;
        bool m_opposingDecelFactorApplied = false;

        // Movement scale factors
        // assuming the event value multipliers are all +1.0
        float m_forwardScale = 1.f;
        float m_backScale = 0.75f;
        float m_leftScale = 1.f;
        float m_rightScale = 1.f;
        float m_sprintAccelScale = 1.5f;
        float m_crouchScale = 0.5f;
        bool m_forwardBackCancelOut = false;
        // This sphere cast determines how far above the charcter's head that an obstruction is detected
        // for allowing them to uncrouch
        float m_uncrouchHeadSphereCastOffset = 0.1f;
        bool m_standPrevented = false;
        bool m_standPreventedViaScript = false;
        AzPhysics::CollisionGroups::Id m_standCollisionGroupId = AzPhysics::CollisionGroups::Id();
        AzPhysics::CollisionGroup m_standCollisionGroup = AzPhysics::CollisionGroup::All;
        AZStd::vector<AZ::EntityId> m_standPreventedEntityIds;
        bool m_standIgnoreDynamicRigidBodies = true;

        // Event value multipliers
        float m_forwardValue = 0.f;
        float m_backValue = 0.f;
        float m_leftValue = 0.f;
        float m_rightValue = 0.f;
        float m_yawValue = 0.f;
        float m_pitchValue = 0.f;
        float m_sprintValue = 0.f;
        float m_crouchValue = 0.f;
        float m_jumpValue = 0.f;

        // Event IDs and action names
        StartingPointInput::InputEventNotificationId m_moveForwardEventId;
        AZStd::string m_strForward = "Forward";
        StartingPointInput::InputEventNotificationId m_moveBackEventId;
        AZStd::string m_strBack = "Back";
        StartingPointInput::InputEventNotificationId m_moveLeftEventId;
        AZStd::string m_strLeft = "Left";
        StartingPointInput::InputEventNotificationId m_moveRightEventId;
        AZStd::string m_strRight = "Right";
        StartingPointInput::InputEventNotificationId m_rotateYawEventId;
        AZStd::string m_strYaw = "Yaw";
        StartingPointInput::InputEventNotificationId m_rotatePitchEventId;
        AZStd::string m_strPitch = "Pitch";
        StartingPointInput::InputEventNotificationId m_sprintEventId;
        AZStd::string m_strSprint = "Sprint";
        StartingPointInput::InputEventNotificationId m_crouchEventId;
        AZStd::string m_strCrouch = "Crouch";
        StartingPointInput::InputEventNotificationId m_jumpEventId;
        AZStd::string m_strJump = "Jump";

        // Array of action names
        AZStd::string* m_inputNames[9] = { &m_strForward, &m_strBack,   &m_strLeft,   &m_strRight, &m_strYaw,
                                           &m_strPitch,   &m_strSprint, &m_strCrouch, &m_strJump };

        // Map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_controlMap = {
            { &m_moveForwardEventId, &m_forwardValue }, { &m_moveBackEventId, &m_backValue }, { &m_moveLeftEventId, &m_leftValue },
            { &m_moveRightEventId, &m_rightValue },     { &m_rotateYawEventId, &m_yawValue }, { &m_rotatePitchEventId, &m_pitchValue },
            { &m_sprintEventId, &m_sprintValue },       { &m_crouchEventId, &m_crouchValue }, { &m_jumpEventId, &m_jumpValue }
        };
    };
} // namespace FirstPersonController
