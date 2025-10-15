/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/FirstPersonExtrasComponentBus.h>
#include <FirstPersonController/FirstPersonControllerComponentBus.h>

#include <Clients/FirstPersonControllerComponent.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/map.h>

#include <StartingPointInput/InputEventNotificationBus.h>

namespace FirstPersonController
{
    class FirstPersonExtrasComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AZ::EntityBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
        , public FirstPersonControllerComponentNotificationBus::Handler
        , public FirstPersonExtrasComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FirstPersonExtrasComponent, "{86d186ce-6065-4cb2-adda-48c630eb5ec4}");

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

        // FirstPersonExtrasRequestBus
        float GetJumpPressedInAirQueueTimeThreshold() const override;
        void SetJumpPressedInAirQueueTimeThreshold(const float& new_jumpPressedInAirQueueTimeThreshold) override;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const bool& tickElseTimestep);

        // FirstPersonExtrasComponentNotificationBus
        void OnPlaceholder();

        // Stores the previous tick deltaTime and previous physics timestep
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevTimestep = 1.f / 60.f;

        // Jump queuing
        void QueueJump(const float& deltaTime, const bool& timestepElseTick);
        bool m_queueJump = false;
        bool m_prevQueueJump = false;
        float m_jumpPressedInAirTimer = 0.f;
        float m_jumpPressedInAirQueueTimeThreshold = 0.35f;

        // Jumping and gravity FirstPersonController attributes
        bool* m_grounded;

        // FirstPersonExtrasComponent object
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;

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
        AZStd::string* m_inputNames[1] = {
            &m_strInteract
        };

        // Map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_controlMap = {
            {&m_interactEventId, &m_interactValue}
        };

        // FirstPersonControllerComponentNotificationBus
        void OnPhysicsTimestepStart(const float& timeStep);
        void OnPhysicsTimestepFinish(const float& timeStep);
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
