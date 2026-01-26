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
        void SetJumpPressedInAirQueueTimeThreshold(const float& new_jumpPressedInAirQueueTimeThreshold) override;
        bool GetJumpHeadTiltEnabled() const override;
        void SetJumpHeadTiltEnabled(const bool& new_jumpHeadTiltEnabled) override;
        float GetHeadAngleJump() const override;
        void SetHeadAngleJump(const float& new_headAngleJump) override;
        float GetHeadAngleLand() const override;
        void SetHeadAngleLand(const float& new_headAngleLand) override;
        float GetDeltaAngleFactorJump() const override;
        void SetDeltaAngleFactorJump(const float& new_deltaAngleFactorJump) override;
        float GetDeltaAngleFactorLand() const override;
        void SetDeltaAngleFactorLand(const float& new_deltaAngleFactorLand) override;
        float GetCompleteHeadLandTime() const override;
        void SetCompleteHeadLandTime(const float& new_completeHeadLandTime) override;
        bool GetSprintFoVEnabled() const override;
        void SetSprintFoVEnabled(const bool& new_sprintFoVEnabled) override;
        float GetSprintFoVLerpTime() const override;
        void SetSprintFoVLerpTime(const float& new_sprintFoVLerpTime) override;
        float GetSprintingFoV() const override;
        void SetSprintingFoV(const float& new_sprintFoV) override;
        float GetWalkingFoV() const override;
        void SetWalkingFoV(const float& new_walkFoV) override;
        bool GetHeadbobEnabled() const override;
        void SetHeadbobEnabled(const bool& new_headbobEnabled) override;
        AZ::EntityId GetHeadbobEntityId() const override;
        void SetHeadbobEntityId(const AZ::EntityId& new_headbobEntityId) override;
        AZ::Vector3 GetCameraTranslationWithoutHeadbob() const override;
        AZ::Vector3 GetPreviousOffset() const override;
        void IgnoreInputs(const bool& ignoreInputs) override;
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

        // NetworkFPCControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnHostActivated(const AZ::EntityId& entityId);

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // FirstPersonExtrasComponentNotificationBus
        void OnJumpFromQueue();

        // Change the camera field of view when sprinting
        void PerformSprintFoV(const float& deltaTime);

        // Jump Head Tilt
        void PerformJumpHeadTilt(const float& deltaTime);

        // FirstPersonControllerComponent and NetworkFPC objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
#ifdef NETWORKFPC
        NetworkFPC* m_networkFPCObject = nullptr;
#else
        bool* m_networkFPCObject = nullptr;
#endif

        // Networking related variables
        bool m_networkFPCEnabled = false;

        // Stores the previous frame tick deltaTime, previous physics timestep, and previous NetworkFPC tick deltaTime
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevTimestep = 1.f / 60.f;
        float m_prevNetworkFPCDeltaTime = 1.f / 60.f;

        // Jump queuing
        void QueueJump(const float& deltaTime, const AZ::u8& tickTimestepNetwork);
        bool m_queueJump = false;
        bool m_prevQueueJump = false;
        float m_jumpPressedInAirTimer = 0.f;
        float m_jumpPressedInAirQueueTimeThreshold = 0.35f;

        // Jumping and gravity FirstPersonController attributes
        bool* m_grounded;

        // Jump Head Tilt
        bool m_jumpHeadTiltEnabled = true;
        bool m_tiltJumped = false;
        bool m_tiltLanded = false;
        bool m_moveHeadDown = true;
        float m_totalHeadAngle = 0.f;
        float m_completeHeadLandTime = 0.4f;
        float m_currentHeadPitchAngle = 0.f;
        float m_deltaAngle = 0.f;
        float m_headAngleJump = 2.5f;
        float m_headAngleLand = 2.f;
        float m_deltaAngleFactorJump = 5.f;
        float m_deltaAngleFactorLand = 8.f;

        // Sprint FoV
        bool m_sprintFoVEnabled = true;
        float m_sprintFoVTimeAccumulator = 0.f;
        float m_sprintFoVLerpTime = 0.5f;
        float m_sprintFoV = 90.f;
        float m_sprintFoVDelta = 2.5f;
        float m_walkFoV = 80.f;

        // Headbob
        void UpdateHeadbob(const float& deltaTime);
        AZ::Vector3 CalculateHeadbobOffset(const float& deltaTime);
        bool m_headbobEnabled = true;
        bool m_isWalking = false;
        bool m_needsHeadbobFallback = false;
        float m_headbobFrequency = 6.15f;
        float m_prevEffectiveFrequency = 6.15f;
        float m_headbobHorizontalAmplitude = 0.01f;
        AZ::Vector3 m_cameraTranslationWithoutHeadbob = AZ::Vector3::CreateZero();
        float m_headbobVerticalAmplitude = 0.03f;
        float m_backwardsFrequencyScale = 0.875f;
        float m_backwardsHorizontalAmplitudeScale = 1.f;
        float m_backwardsVerticalAmplitudeScale = 1.f;
        float m_crouchFrequencyScale = 0.875f;
        float m_crouchHorizontalAmplitudeScale = 0.875f;
        float m_crouchVerticalAmplitudeScale = 0.875f;
        float m_sprintFrequencyScale = 1.25f;
        float m_sprintHorizontalAmplitudeScale = 1.125f;
        float m_sprintVerticalAmplitudeScale = 1.125f;
        float m_headbobAttenuation = 0.25f;
        float m_walkingTime = 0.f;
        AZ::Vector3 m_originalCameraTranslation = AZ::Vector3::CreateZero();
        AZ::Vector3 m_headbobOffset = AZ::Vector3::CreateZero();
        AZ::Vector3 m_previousOffset = AZ::Vector3::CreateZero();
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
        void OnPhysicsTimestepStart(const float& timeStep, const AZ::EntityId& entityId);
        void OnPhysicsTimestepFinish(const float& timeStep, const AZ::EntityId& entityId);
        void OnNetworkFPCTickStart(const float& deltaTime, const AZ::EntityId& entityId);
        void OnNetworkFPCTickFinish(const float& deltaTime, const AZ::EntityId& entityId);
        void OnNetworkFPCAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnNetworkFPCHostActivated(const AZ::EntityId& entityId);
        void OnFPCActivated(const AZ::EntityId& entityId);
        void OnGroundHit(const float& fellVelocity, const AZ::EntityId& entityId);
        void OnGroundSoonHit(const float& soonFellVelocity, const AZ::EntityId& entityId);
        void OnUngrounded(const AZ::EntityId& entityId);
        void OnStartedFalling(const AZ::EntityId& entityId);
        void OnJumpApogeeReached(const AZ::EntityId& entityId);
        void OnStartedMoving(const AZ::EntityId& entityId);
        void OnTargetVelocityReached(const AZ::EntityId& entityId);
        void OnStopped(const AZ::EntityId& entityId);
        void OnTopWalkSpeedReached(const AZ::EntityId& entityId);
        void OnTopSprintSpeedReached(const AZ::EntityId& entityId);
        void OnHeadHit(const AZ::EntityId& entityId);
        void OnCharacterShapecastHitSomething(const AZStd::vector<AzPhysics::SceneQueryHit> characterHits, const AZ::EntityId& entityId);
        void OnVelocityXYObstructed(const AZ::EntityId& entityId);
        void OnCharacterGravityObstructed(const AZ::EntityId& entityId);
        void OnCrouched(const AZ::EntityId& entityId);
        void OnStoodUp(const AZ::EntityId& entityId);
        void OnStoodUpFromJump(const AZ::EntityId& entityId);
        void OnStandPrevented(const AZ::EntityId& entityId);
        void OnStartedCrouching(const AZ::EntityId& entityId);
        void OnStartedStanding(const AZ::EntityId& entityId);
        void OnFirstJump(const AZ::EntityId& entityId);
        void OnFinalJump(const AZ::EntityId& entityId);
        void OnStaminaCapped(const AZ::EntityId& entityId);
        void OnStaminaReachedZero(const AZ::EntityId& entityId);
        void OnSprintStarted(const AZ::EntityId& entityId);
        void OnSprintStopped(const AZ::EntityId& entityId);
        void OnCooldownStarted(const AZ::EntityId& entityId);
        void OnCooldownDone(const AZ::EntityId& entityId);
    };
} // namespace FirstPersonController
