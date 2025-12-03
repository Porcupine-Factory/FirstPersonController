#pragma once
#include <FirstPersonController/NetworkFPCControllerBus.h>

#include <Source/AutoGen/NetworkFPC.AutoComponent.h>

#include <Clients/FirstPersonControllerComponent.h>
#include <Clients/FirstPersonExtrasComponent.h>

namespace FirstPersonController
{
    class FirstPersonControllerComponent;

    class FirstPersonExtrasComponent;

    class NetworkFPCController
        : public NetworkFPCControllerBase
        , public NetworkFPCControllerRequestBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
        friend class FirstPersonControllerComponent;
        friend class FirstPersonExtrasComponent;

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
        bool GetIsNetEntityAutonomous() const override;
        bool GetEnabled() const override;
        void SetEnabled(const bool& new_enabled) override;

        // AZ::InputEventNotificationBus interface
        void OnPressed(float value) override;
        void OnReleased(float value) override;
        void OnHeld(float value) override;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        // NetworkFPCControllerNotificationBus
        void OnNetworkTick(const float& deltaTime);

        // Keep track of the previous deltaTime for averagin
        float m_prevDeltaTime = 1.f / 60.f;

        // EnableNetworkFPC Changed Event
        AZ::Event<bool>::Handler m_enableNetworkFPCChangedEvent;
        void OnEnableNetworkFPCChanged(const bool& enable);
        bool m_disabled = false;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;
        bool m_isAuthority = false;

        // FirstPersonControllerComponent and FirstPersonExtrasComponent objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
        FirstPersonExtrasComponent* m_firstPersonExtrasObject = nullptr;

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
