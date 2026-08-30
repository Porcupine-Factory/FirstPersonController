/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#include <FirstPersonController/FirstPersonExtrasComponentBus.h>

#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/std/containers/map.h>

#include <AzFramework/Components/CameraBus.h>

#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    class NetworkFPC;

    class FirstPersonExtrasComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AZ::EntityBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
        , public FirstPersonControllerComponentNotificationBus::Handler
#ifdef NETWORKFPC
        , public NetworkFPCControllerNotificationBus::Handler
#endif
        , public FirstPersonExtrasComponentRequestBus::Handler
        , public Camera::CameraNotificationBus::Handler
    {
        friend class FirstPersonControllerComponent;
        friend class CameraCoupledChildComponent;
        friend class NetworkFPCController;

    public:
        AZ_COMPONENT(FirstPersonExtrasComponent, "{86d186ce-6065-4cb2-adda-48c630eb5ec4}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::EntityBus interface
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

        void OnActiveViewChanged(const AZ::EntityId& activeEntityId) override;

        // FirstPersonExtrasRequestBus
        float GetJumpPressedInAirQueueTimeThreshold() const override;
        void SetJumpPressedInAirQueueTimeThreshold(const float jumpPressedInAirQueueTimeThreshold) override;
        float GetJumpStaminaEquivalentSprintTime() const override;
        void SetJumpStaminaEquivalentSprintTime(const float jumpStaminaEquivalentSprintTime) override;
        float GetJumpExhaustionFactor() const override;
        void SetJumpExhaustionFactor(const float jumpExhaustionFactor) override;
        float GetJumpDefaultHoldDistance() const override;
        void SetJumpDefaultHoldDistance(const float jumpDefaultHoldDistance) override;
        float GetJumpDefaultInitialVelocity() const override;
        void SetJumpDefaultInitialVelocity(const float jumpDefaultInitialVelocity) override;
        bool GetJumpHeadTiltEnabled() const override;
        void SetJumpHeadTiltEnabled(const bool jumpHeadTiltEnabled) override;
        float GetHeadAngleJump() const override;
        void SetHeadAngleJump(const float headAngleJump) override;
        float GetHeadAngleLand() const override;
        void SetHeadAngleLand(const float headAngleLand) override;
        float GetDeltaAngleFactorJump() const override;
        void SetDeltaAngleFactorJump(const float deltaAngleFactorJump) override;
        float GetDeltaAngleFactorLand() const override;
        void SetDeltaAngleFactorLand(const float deltaAngleFactorLand) override;
        float GetCompleteHeadLandVelocity() const override;
        void SetCompleteHeadLandVelocity(const float completeHeadLandVelocity) override;
        bool GetSprintFoVEnabled() const override;
        void SetSprintFoVEnabled(const bool sprintFoVEnabled) override;
        float GetSprintFoVLerpTime() const override;
        void SetSprintFoVLerpTime(const float sprintFoVLerpTime) override;
        float GetSprintingFoV() const override;
        void SetSprintingFoV(const float sprintFoV) override;
        float GetWalkingFoV() const override;
        void SetWalkingFoV(const float walkFoV) override;
        bool GetHeadbobEnabled() const override;
        void SetHeadbobEnabled(const bool headbobEnabled) override;
        bool GetHeadbobStartingDirection() const override;
        void SetHeadbobStartingDirection(const bool headbobStartingDirection) override;
        float GetHeadbobMaxFrequency() const override;
        void SetHeadbobMaxFrequency(const float headbobMaxFrequency) override;
        float GetHeadbobMaxVerticalAmplitude() const override;
        void SetHeadbobMaxVerticalAmplitude(const float headbobMaxVerticalAmplitude) override;
        float GetHeadbobMaxHorizontalAmplitude() const override;
        void SetHeadbobMaxHorizontalAmplitude(const float headbobMaxHorizontalAmplitude) override;
        float GetHeadbobOverallIntensity() const override;
        void SetHeadbobOverallIntensity(const float headbobOverallIntensity) override;
        float GetHeadbobSmoothTime() const override;
        void SetHeadbobSmoothTime(const float headbobSmoothTime) override;
        float GetHeadbobRealism() const override;
        void SetHeadbobRealism(const float headbobRealism) override;
        float GetHeadbobFootstepSharpness() const override;
        void SetHeadbobFootstepSharpness(const float headbobFootstepSharpness) override;
        float GetHeadbobAlternatingStepDifference() const override;
        void SetHeadbobAlternatingStepDifference(const float headbobAlternatingStepDifference) override;
        float GetHeadbobHorizontalSwayImbalance() const override;
        void SetHeadbobHorizontalSwayImbalance(const float headbobHorizontalSwayImbalance) override;
        float GetHeadbobHorizontalSwayFlatness() const override;
        void SetHeadbobHorizontalSwayFlatness(const float headbobHorizontalSwayFlatness) override;
        float GetHeadbobFootstepAcceleration() const override;
        void SetHeadbobFootstepAcceleration(const float headbobFootstepAcceleration) override;
        float GetHeadbobMaxPitchAmplitude() const override;
        void SetHeadbobMaxPitchAmplitude(const float headbobMaxPitchAmplitude) override;
        float GetHeadbobMaxRollAmplitude() const override;
        void SetHeadbobMaxRollAmplitude(const float headbobMaxRollAmplitude) override;
        float GetHeadbobMaxYawAmplitude() const override;
        void SetHeadbobMaxYawAmplitude(const float headbobMaxYawAmplitude) override;
        float GetHeadbobStepVariationOverTime() const override;
        void SetHeadbobStepVariationOverTime(const float headbobStepVariationOverTime) override;
        float GetHeadbobVerticalSprintScale() const override;
        void SetHeadbobVerticalSprintScale(const float headbobVerticalSprintScale) override;
        float GetHeadbobHorizontalSprintScale() const override;
        void SetHeadbobHorizontalSprintScale(const float headbobHorizontalSprintScale) override;
        float GetHeadbobRotationSprintScale() const override;
        void SetHeadbobRotationSprintScale(const float headbobRotationSprintScale) override;
        float GetHeadbobRotationCrouchScale() const override;
        void SetHeadbobRotationCrouchScale(const float headbobRotationCrouchScale) override;
        float GetHeadbobVerticalCrouchScale() const override;
        void SetHeadbobVerticalCrouchScale(const float headbobVerticalCrouchScale) override;
        float GetHeadbobHorizontalCrouchScale() const override;
        void SetHeadbobHorizontalCrouchScale(const float headbobHorizontalCrouchScale) override;
        float GetHeadbobLastStepStrength() const override;
        AZ::Vector3 GetCameraTranslationWithoutHeadbob() const override;
        AZ::Vector3 GetPreviousOffset() const override;
        void IgnoreInputs(const bool ignoreInputs) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // Assigns a camera to m_cameraEntityId if none is specified
        void OnCameraAdded(const AZ::EntityId& cameraId);
        AZ::Entity* GetActiveCamera() const;
        AZ::Entity* GetEntityPtr(AZ::EntityId pointer) const;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;
        int GetTickOrder() override;

        // NetworkFPCControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnHostActivated(const AZ::EntityId& entityId);
        void OnNonAutonomousClientActivated(const AZ::EntityId& entityId);

        // Called on each tick
        void ProcessInput(const float deltaTime, const AZ::u8 tickTimestepNetwork);

        // Change the camera field of view when sprinting
        void PerformSprintFoV(const float deltaTime);
        bool GetSprinting();

        // Jump Head Tilt
        void PerformJumpHeadTilt(const float deltaTime);

        // FirstPersonControllerComponent and NetworkFPC objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
#ifdef NETWORKFPC
        NetworkFPC* m_networkFPCObject = nullptr;
#else
        bool* m_networkFPCObject = nullptr;
#endif

        // Networking related variables
        bool m_networkFPCEnabled = false;

        // Jump queuing
        void QueueJump(const float deltaTime, const AZ::u8 tickTimestepNetwork);
        bool m_queueJump = false;
        bool m_prevQueueJump = false;
        float m_jumpPressedInAirTimer = 0.f;
        float m_jumpPressedInAirQueueTimeThreshold = 0.35f;

        // Jump Uses Stamina
        float m_jumpStaminaEquivalentSprintTime = 1.9f;

        // Exhausted jumping
        float m_jumpExhaustionFactor = 0.4f;
        float m_jumpDefaultHoldDistance = 0.8f;
        float m_jumpDefaultInitialVelocity = 6.f;

        // Jumping and gravity FirstPersonController attributes
        bool* m_grounded;

        // Jump Head Tilt
        bool m_jumpHeadTiltEnabled = true;
        bool m_tiltJumped = false;
        bool m_tiltLanded = false;
        bool m_moveHeadDown = true;
        float m_totalHeadAngle = 0.f;
        float m_completeHeadLandVelocity = 8.1f;
        float m_currentHeadPitchAngle = 0.f;
        float m_deltaAngle = 0.f;
        float m_headAngleJump = 1.6f;
        float m_headAngleLand = 1.f;
        float m_deltaAngleFactorJump = 12.f;
        float m_deltaAngleFactorLand = 10.f;

        // Sprint FoV
        bool m_sprintFoVEnabled = true;
        bool m_sprintingObstructedCheck[16] = {};
        AZ::u8 m_sprintingObstructedIndex = 0;
        float m_sprintFoVTimeAccumulator = 0.f;
        float m_sprintFoVLerpTime = 0.5f;
        float m_sprintFoV = 90.f;
        float m_sprintFoVDelta = 2.5f;
        float m_walkFoV = 80.f;

        // Headbob
        void UpdateHeadbob(const float deltaTime);
        AZ::Vector3 CalculateHeadbobOffset(const float deltaTime);
        bool m_headbobEnabled = true;
        bool m_isWalking = false;
        bool m_cameraChildOfCharacter = true;
        bool m_needsHeadbobFallback = false;
        bool m_headbobStartingDirection = true;
        float m_headbobMaxFrequency = 1.23f;
        float m_headbobNormalizedVerticalShape = 0.f;
        float m_headbobSmoothedVerticalShape = 0.f;
        float m_prevHeadbobSmoothedVerticalShape = 0.f;
        bool m_stepTaken = false;
        float m_headbobLastStepStrength = 1.f;
        float m_headbobMaxHorizontalAmplitude = 0.0125f;
        float m_headbobMaxVerticalAmplitude = 0.038f;
        float m_headbobOverallIntensity = 1.f;
        float m_headbobSmoothTime = 0.05f;
        float m_headbobRealism = 0.25f;
        float m_headbobFootstepSharpness = 0.2f;
        float m_headbobAlternatingStepDifference = 0.27f;
        float m_headbobHorizontalSwayImbalance = 0.14f;
        float m_headbobHorizontalSwayFlatness = 0.05f;
        float m_headbobFootstepAcceleration = 0.6f;
        float m_headbobMaxPitchAmplitude = 0.4f;
        float m_headbobMaxRollAmplitude = 0.5f;
        float m_headbobMaxYawAmplitude = 0.8f;
        float m_headbobStepVariationOverTime = 0.03f;
        float m_headbobVerticalSprintScale = 1.6f;
        float m_headbobHorizontalSprintScale = 0.85f;
        float m_headbobRotationSprintScale = 1.6f;
        float m_headbobVerticalCrouchScale = 0.85f;
        float m_headbobHorizontalCrouchScale = 1.5f;
        float m_headbobRotationCrouchScale = 1.f;
        float m_headbobVerticalShapePeak = 1.f;
        float m_headbobHorizontalShapePeak = 1.f;
        float m_headbobForwardShapePeak = 1.f;
        float m_headbobPhase = 0.f;
        float CalculateHeadbobVerticalShape(const float phase) const;
        float CalculateHeadbobHorizontalShape(const float phase) const;
        float CalculateHeadbobForwardShape(const float phase) const;
        void UpdateHeadbobShapePeaks();
        AZ::u32 OnHeadbobRealismChanged();
        bool GetHeadbobEnabledAndRealismGreaterThanZero() const;
        AZ::Vector3 m_cameraTranslationWithoutHeadbob = AZ::Vector3::CreateZero();
        AZ::Vector3 m_originalCameraTranslation = AZ::Vector3::CreateZero();
        AZ::Vector3 m_headbobOffset = AZ::Vector3::CreateZero();
        AZ::Vector3 m_prevHeadbobOffset = AZ::Vector3::CreateZero();
        AZ::Vector3 m_smoothedHeadbobOffset = AZ::Vector3::CreateZero();
        AZ::Quaternion m_headbobRotationOffset = AZ::Quaternion::CreateIdentity();
        AZ::Quaternion m_prevHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
        AZ::Quaternion m_smoothedHeadbobRotationOffset = AZ::Quaternion::CreateIdentity();
        AZ::EntityId m_cameraEntityId = AZ::EntityId();
        AZ::Entity* m_cameraEntityPtr = nullptr;

        // FirstPersonController event value multipliers
        float* m_jumpValue = nullptr;
        bool* m_scriptJump = nullptr;
        float m_prevJumpValue = 0.f;

        // Event value multipliers
        float m_interactValue = 0.f;

        // Event IDs and action names
        StartingPointInput::InputEventNotificationId m_interactEventId;
        AZStd::string m_strInteract = "Interact";

        // Array of action names
        AZStd::string* m_inputNames[1] = { &m_strInteract };

        // Map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_controlMap = { { &m_interactEventId, &m_interactValue } };

        // FirstPersonControllerComponentNotificationBus
        void OnPhysicsTimestepStart(const float timeStep, const AZ::EntityId& entityId);
        void OnPhysicsTimestepFinish(const float timeStep, const AZ::EntityId& entityId);
        void OnNetworkFPCTickStart(const float deltaTime, const AZ::EntityId& entityId);
        void OnNetworkFPCTickFinish(const float deltaTime, const AZ::EntityId& entityId);
        void OnNetworkFPCAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnNetworkFPCHostActivated(const AZ::EntityId& entityId);
        void OnNetworkFPCNonAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnFPCActivated(const AZ::EntityId& entityId);
        void OnGroundHit(const float fellVelocity);
        void OnGroundSoonHit(const float soonFellVelocity);
        void OnUngrounded();
        void OnStartedFalling();
        void OnJumpApogeeReached();
        void OnStartedMoving();
        void OnTargetVelocityReached();
        void OnStopped();
        void OnTopWalkSpeedReached();
        void OnTopSprintSpeedReached();
        void OnHeadHit();
        void OnCharacterShapecastHitSomething(const AZStd::vector<AzPhysics::SceneQueryHit> characterHits);
        void OnVelocityXYObstructed();
        void OnCharacterGravityObstructed();
        void OnCrouched();
        void OnStoodUp();
        void OnStoodUpFromJump();
        void OnStandPrevented();
        void OnStartedCrouching();
        void OnStartedStanding();
        void OnFirstJump();
        void OnFinalJump();
        void OnStaminaCapped();
        void OnStaminaReachedZero();
        void OnSprintStarted();
        void OnSprintStopped();
        void OnCooldownStarted();
        void OnCooldownDone();
    };
} // namespace FirstPersonController
