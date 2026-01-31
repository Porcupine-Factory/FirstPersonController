/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/NetworkFPCControllerBus.h>

#include <Source/AutoGen/NetworkFPC.AutoComponent.h>

#include <Clients/FirstPersonControllerComponent.h>
#include <Clients/FirstPersonExtrasComponent.h>

#include <Integration/ActorComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>

#include <Multiplayer/Components/NetBindComponent.h>

namespace EMotionFX
{
    class AnimGraphComponentNetworkRequests;
    namespace Integration
    {
        class ActorComponentRequests;
        class AnimGraphComponentRequests;
    } // namespace Integration
} // namespace EMotionFX

namespace FirstPersonController
{
    // This is not documented, you kind of have to jump into EMotionFX's private headers to find this, invalid parameter index values are
    // max size_t See InvalidIndex in Gems\EMotionFX\Code\EMotionFX\Source\EMotionFXConfig.h
    constexpr size_t InvalidParamIndex = 0xffffffffffffffff;

    class FirstPersonControllerComponent;

    class FirstPersonExtrasComponent;

    class NetworkFPC
        : public NetworkFPCBase
        , public EMotionFX::Integration::ActorComponentNotificationBus::Handler
        , public EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
    {
    public:
        AZ_MULTIPLAYER_COMPONENT(FirstPersonController::NetworkFPC, s_networkFPCConcreteUuid, FirstPersonController::NetworkFPCBase);

        static void Reflect(AZ::ReflectContext* context);

        NetworkFPC();

        void OnInit() override;
        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

    private:
        void OnPreRender(float deltaTime);

        // EnableAnimationNetworkFPC Changed Event
        AZ::Event<bool>::Handler m_enableNetworkAnimationChangedEvent;
        void OnEnableNetworkAnimationChanged(const bool& enable);

        //! EMotionFX::Integration::ActorComponentNotificationBus::Handler
        //! @{
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;
        //! @}

        //! EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
        //! @{
        void OnAnimGraphInstanceCreated(EMotionFX::AnimGraphInstance* animGraphInstance) override;
        //! @}

        // FirstPersonControllerComponent and FirstPersonExtrasComponent objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;

        // Network animation members
        Multiplayer::EntityPreRenderEvent::Handler m_preRenderEventHandler;

        EMotionFX::Integration::ActorComponentRequests* m_actorRequests = nullptr;
        EMotionFX::AnimGraphComponentNetworkRequests* m_networkRequests = nullptr;
        EMotionFX::Integration::AnimGraphComponentRequests* m_animationGraph = nullptr;

        bool m_animationChildFound = false;

        AZ::EntityId m_animationEntityId = AZ::EntityId();

        void DetectAnimationChild();
        void SetupAnimationConnections(const AZ::EntityId& targetId);

        bool m_paramIdsSet = false;
        size_t m_walkSpeedParamId = InvalidParamIndex;
        size_t m_sprintParamId = InvalidParamIndex;
        size_t m_crouchToStandParamId = InvalidParamIndex;
        size_t m_crouchParamId = InvalidParamIndex;
        size_t m_standToCrouchParamId = InvalidParamIndex;
        size_t m_jumpStartParamId = InvalidParamIndex;
        size_t m_fallParamId = InvalidParamIndex;
        size_t m_landParamId = InvalidParamIndex;
        size_t m_groundedParamId = InvalidParamIndex;

        // NOTE: Make sure to add any new param Ids to this param Ids array
        size_t* m_paramIds[9] = { &m_walkSpeedParamId, &m_sprintParamId,        &m_crouchToStandParamId,
                                  &m_crouchParamId,    &m_standToCrouchParamId, &m_jumpStartParamId,
                                  &m_fallParamId,      &m_landParamId,          &m_groundedParamId };
    };

    class NetworkFPCController
        : public NetworkFPCControllerBase
        , public NetworkFPCControllerRequestBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
        friend class FirstPersonControllerComponent;
        friend class FirstPersonExtrasComponent;
        friend class CameraCoupledChildComponent;

    public:
        explicit NetworkFPCController(NetworkFPC& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        //! Common input creation logic for the NetworkInput.
        //! Fill out the input struct and the MultiplayerInputDriver will send the input data over the network
        //!    to ensure it's processed.
        //! @param input  input structure which to store input data for sending to the authority
        //! @param deltaTime amount of time to integrate the provided inputs over
        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;

        //! Common input processing logic for the NetworkInput.
        //! @param input  input structure to process
        //! @param deltaTime amount of time to integrate the provided inputs over
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;

        // NetworkFPCControllerRequestBus
        void TryAddVelocityForNetworkTick(const AZ::Vector3& tryVelocity, const float& deltaTime) override;
        bool GetAllowAllMovementInputs() const override;
        void SetAllowAllMovementInputs(const bool& new_allowAllMovementInputs) override;
        bool GetAllowRotationInputs() const override;
        void SetAllowRotationInputs(const bool& new_allowRotationInputs) override;
        AZ::TimeMs GetHostTimeMs() const override;
        bool GetEnabled() const override;
        void SetEnabled(const bool& new_enabled) override;
        bool GetIsNetEntityRoleAuthority() const override;

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

    private:
        void OnPreRender(float deltaTime);

        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // NetworkFPCControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnAutonomousClientActivated(const AZ::EntityId& entityId);
        void OnHostActivated(const AZ::EntityId& entityId);
        void OnNonAutonomousClientActivated(const AZ::EntityId& entityId);

        // Used to initialize Network Properties from initial values in the First Person Controller component
        bool m_init = true;

        // Keep track of the previous deltaTime for averagin
        float m_prevDeltaTime = 1.f / 60.f;

        // EnableNetworkFPC Changed Event
        AZ::Event<bool>::Handler m_enableNetworkFPCChangedEvent;
        void OnEnableNetworkFPCChanged(const bool& enable);
        bool m_disabled = false;

        // Used to allow or prevent all player character inputs from going to the server (e.g. in menus)
        bool m_allowAllMovementInputs = true;

        // Used to allow or prevent the rotation inputs from being applied to the character (e.g. in menus)
        bool m_allowRotationInputs = true;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // FirstPersonControllerComponent and FirstPersonExtrasComponent objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
        FirstPersonExtrasComponent* m_firstPersonExtrasObject = nullptr;

        // Used in determining if the character was recently grounded
        bool m_groundedRecently = true;

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
