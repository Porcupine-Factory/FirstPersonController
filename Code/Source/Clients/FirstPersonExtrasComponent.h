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
        , public NetworkFPCControllerNotificationBus::Handler
        , public FirstPersonExtrasComponentRequestBus::Handler
        , public Camera::CameraNotificationBus::Handler
    {
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
        bool GetEnableHeadbob() const;
        AZ::EntityId GetHeadbobEntityId() const override;
        void SetHeadbobEntityId(const AZ::EntityId&) override;
        void NetworkFPCEnabledIgnoreInputs() override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        AZ::Entity* GetActiveCamera() const;
        AZ::Entity* GetEntityPtr(AZ::EntityId pointer) const;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // NetworkFPCControllerNotificationBus
        void OnNetworkTick(const float& deltaTime);

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // FirstPersonExtrasComponentNotificationBus
        void OnJumpFromQueue();

        // FirstPersonControllerComponent and NetworkFPC objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
        NetworkFPC* m_networkFPCObject = nullptr;

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

        // Headbob
        void SetHeadbobEntity(const AZ::EntityId& id);
        void UpdateHeadbob(const float& deltaTime);
        AZ::Vector3 CalculateHeadbobOffset(const float& deltaTime);
        bool m_enableHeadbob = false;
        bool m_isWalking = false;
        bool m_needsHeadbobFallback = false;
        float m_headbobFrequency = 6.15f;
        float m_headbobHorizontalAmplitude = 0.01f;
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
        AZ::EntityId m_headbobEntityId = AZ::EntityId();
        AZ::Entity* m_headbobEntityPtr = nullptr;

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
        void OnPhysicsTimestepStart(const float& timeStep);
        void OnPhysicsTimestepFinish(const float& timeStep);
        void OnNetworkFPCTick(const float& deltaTime);
        void OnGroundHit(const float& fellDistance);
        void OnGroundSoonHit(const float& soonFellDistance);
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
