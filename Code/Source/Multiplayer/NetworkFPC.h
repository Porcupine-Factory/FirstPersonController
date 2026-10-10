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
        friend class NetworkFPCController;

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
        void OnEnableNetworkAnimationChanged(const bool enable);

        //! EMotionFX::Integration::ActorComponentNotificationBus::Handler
        //! @{
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;
        //! @}

        //! EMotionFX::Integration::AnimGraphComponentNotificationBus::Handler
        //! @{
        void OnAnimGraphInstanceCreated(EMotionFX::AnimGraphInstance* animGraphInstance) override;
        //! @}

        // FirstPersonControllerComponent object
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

        // Calculated camera pitch angle
        float m_cameraPitch = 0.f;

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
        size_t m_lookUpDownParamId = InvalidParamIndex;

        // NOTE: Make sure to add any new param Ids to this param Ids array
        size_t* m_paramIds[10] = { &m_walkSpeedParamId,     &m_sprintParamId,    &m_crouchToStandParamId, &m_crouchParamId,
                                   &m_standToCrouchParamId, &m_jumpStartParamId, &m_fallParamId,          &m_landParamId,
                                   &m_groundedParamId,      &m_lookUpDownParamId };
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
        void TryAddVelocityForNetworkTick(const AZ::Vector3& tryVelocity, const float deltaTime) override;
        bool GetAllowActionInputs() const override;
        void SetAllowActionInputs(const bool allowActionInputs) override;
        bool GetAllowRotationInputs() const override;
        void SetAllowRotationInputs(const bool allowRotationInputs) override;
        AZ::TimeMs GetHostTimeMs() const override;
        bool GetEnabled() const override;
        void SetEnabled(const bool enabled) override;
        bool GetIsNetEntityRoleAuthority() const override;
        float GetInteractInputValue() const override;
        float GetAttackInputValue() const override;
        float GetBlockInputValue() const override;
        float GetReloadInputValue() const override;
        float GetNextWeaponInputValue() const override;
        float GetPrevWeaponInputValue() const override;
        AZStd::string GetInteractInputName() const override;
        void SetInteractInputName(const AZStd::string& strInteract) override;
        AZStd::string GetAttackInputName() const override;
        void SetAttackInputName(const AZStd::string& strAttack) override;
        AZStd::string GetBlockInputName() const override;
        void SetBlockInputName(const AZStd::string& strBlock) override;
        AZStd::string GetReloadInputName() const override;
        void SetReloadInputName(const AZStd::string& strReload) override;
        AZStd::string GetNextWeaponInputName() const override;
        void SetNextWeaponInputName(const AZStd::string& strNextWeapon) override;
        AZStd::string GetPrevWeaponInputName() const override;
        void SetPrevWeaponInputName(const AZStd::string& strPrevWeapon) override;

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

#if AZ_TRAIT_SERVER
        void HandleObtainParentNetEntityId(AzNetworking::IConnection* invokingConnection, const AZStd::string& strNetEntityId) override;
#endif

    private:
        void OnPreRender(float deltaTime);

        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // Connect and disconnect events
        void OnConnectionAcquired();
        void OnEndpointDisconnected();
        Multiplayer::ConnectionAcquiredEvent::Handler m_connectionAcquiredHandler = Multiplayer::ConnectionAcquiredEvent::Handler(
            [this](Multiplayer::MultiplayerAgentDatum)
            {
                this->OnConnectionAcquired();
            });
        Multiplayer::EndpointDisconnectedEvent::Handler m_endpointDisconnectedHandler = Multiplayer::EndpointDisconnectedEvent::Handler(
            [this](Multiplayer::MultiplayerAgentType)
            {
                this->OnEndpointDisconnected();
            });

        // Used to initialize Network Properties from initial values in the First Person Controller component
        bool m_init = true;

        // EnableNetworkFPC Changed Event
        AZ::Event<bool>::Handler m_enableNetworkFPCChangedEvent;
        AZ::Event<AZStd::vector<AZStd::string>>::Handler m_playerStringNetEntityIdsChangedEvent;
        AZ::Event<AZStd::vector<AZStd::string>>::Handler m_botStringNetEntityIdsChangedEvent;
        void OnEnableNetworkFPCChanged(const bool enable);
        void OnPlayerStringNetEntityIdsChanged(const AZStd::vector<AZStd::string>& playerStringNetEntityIds);
        void OnBotStringNetEntityIdsChanged(const AZStd::vector<AZStd::string>& botStringNetEntityIds);
        bool m_disabled = false;

        // Used to allow or prevent action inputs from going to the server (e.g. in menus)
        bool m_allowActionInputs = true;

        // Used to allow or prevent the rotation inputs from being applied to the character (e.g. in menus)
        bool m_allowRotationInputs = true;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // FirstPersonControllerComponent and FirstPersonExtrasComponent objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
        FirstPersonExtrasComponent* m_firstPersonExtrasObject = nullptr;
        NetworkFPC* m_networkFPCObject = nullptr;

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
        float m_interactValue = 0.f;
        float m_prevInteractValue = 0.f;
        float m_attackValue = 0.f;
        float m_prevAttackValue = 0.f;
        float m_blockValue = 0.f;
        float m_prevBlockValue = 0.f;
        float m_reloadValue = 0.f;
        float m_prevReloadValue = 0.f;
        float m_nextWeaponValue = 0.f;
        float m_prevNextWeaponValue = 0.f;
        float m_prevWeaponValue = 0.f;
        float m_prevPrevWeaponValue = 0.f;

        // Event IDs and input names
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
        StartingPointInput::InputEventNotificationId m_interactEventId;
        AZStd::string m_strInteract = "Interact";
        StartingPointInput::InputEventNotificationId m_attackEventId;
        AZStd::string m_strAttack = "Attack";
        StartingPointInput::InputEventNotificationId m_blockEventId;
        AZStd::string m_strBlock = "Block";
        StartingPointInput::InputEventNotificationId m_reloadEventId;
        AZStd::string m_strReload = "Reload";
        StartingPointInput::InputEventNotificationId m_nextWeaponEventId;
        AZStd::string m_strNextWeapon = "NextWeapon";
        StartingPointInput::InputEventNotificationId m_prevWeaponEventId;
        AZStd::string m_strPrevWeapon = "PrevWeapon";

        // Array of input names
        AZStd::string* m_inputNames[15] = { &m_strForward, &m_strBack,   &m_strLeft,   &m_strRight,      &m_strYaw,
                                            &m_strPitch,   &m_strSprint, &m_strCrouch, &m_strJump,       &m_strInteract,
                                            &m_strAttack,  &m_strBlock,  &m_strReload, &m_strNextWeapon, &m_strPrevWeapon };

        // Map of event IDs and event value multipliers
        AZStd::map<StartingPointInput::InputEventNotificationId*, float*> m_controlMap = {
            { &m_moveForwardEventId, &m_forwardValue },
            { &m_moveBackEventId, &m_backValue },
            { &m_moveLeftEventId, &m_leftValue },
            { &m_moveRightEventId, &m_rightValue },
            { &m_rotateYawEventId, &m_yawValue },
            { &m_rotatePitchEventId, &m_pitchValue },
            { &m_sprintEventId, &m_sprintValue },
            { &m_crouchEventId, &m_crouchValue },
            { &m_jumpEventId, &m_jumpValue },
            { &m_interactEventId, &m_interactValue },
            { &m_attackEventId, &m_attackValue },
            { &m_blockEventId, &m_blockValue },
            { &m_reloadEventId, &m_reloadValue },
            { &m_nextWeaponEventId, &m_nextWeaponValue },
            { &m_prevWeaponEventId, &m_prevWeaponValue },
        };
    };
} // namespace FirstPersonController
