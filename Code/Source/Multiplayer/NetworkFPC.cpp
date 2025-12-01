#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    NetworkFPCController::NetworkFPCController(NetworkFPC& parent)
        : NetworkFPCControllerBase(parent)
        , m_enableNetworkFPCChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkFPCChanged(enable);
              })
    {
    }

    void NetworkFPCController::AssignConnectInputEvents()
    {
        // Disconnect prior to connecting since this may be a reassignment
        InputEventNotificationBus::MultiHandler::BusDisconnect();

        if (m_controlMap.size() != (sizeof(m_inputNames) / sizeof(AZStd::string*)))
        {
            AZ_Error("NetworkFPC Controller", false, "Number of input IDs not equal to number of input names!");
        }
        else
        {
            const AZ::u8 size = sizeof(m_inputNames) / sizeof(AZStd::string*);

            for (AZ::u8 i = 0; i < size; ++i)
                m_inputNames[i] = m_firstPersonControllerObject->m_inputNames[i];

            for (auto& it_event : m_controlMap)
            {
                *(it_event.first) = StartingPointInput::InputEventNotificationId(
                    (m_inputNames[std::distance(m_controlMap.begin(), m_controlMap.find(it_event.first))])->c_str());
                InputEventNotificationBus::MultiHandler::BusConnect(*(it_event.first));
            }
        }
    }

    void NetworkFPCController::OnPressed(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Pressed", it_event.first->ToString().c_str());
            }
        }
    }

    void NetworkFPCController::OnReleased(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first))
            {
                *(it_event.second) = value;
                // print the local user ID and the action name CRC
                // AZ_Printf("Released", it_event.first->ToString().c_str());
            }
        }
    }

    void NetworkFPCController::OnHeld(float value)
    {
        const InputEventNotificationId* inputId = InputEventNotificationBus::GetCurrentBusId();
        if (inputId == nullptr)
            return;

        if (*inputId == m_rotateYawEventId)
        {
            m_yawValue += value;
        }
        else if (*inputId == m_rotatePitchEventId)
        {
            m_pitchValue += value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if (*inputId == m_sprintEventId)
        {
            m_sprintValue = value;
        }
    }

    void NetworkFPCController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkFPCControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkFPC change events
        EnableNetworkFPCAddEvent(m_enableNetworkFPCChangedEvent);

        // Get access to the FirstPersonControllerComponent and FirstPersonExtrasComponent objects and their members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
        m_firstPersonExtrasObject = entity->FindComponent<FirstPersonExtrasComponent>();
        m_firstPersonControllerObject->m_networkFPCEnabled = GetEnableNetworkFPC();
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = GetEnableNetworkFPC();

        m_firstPersonControllerObject->NetworkFPCEnabledIgnoreInputs();
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->NetworkFPCEnabledIgnoreInputs();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_firstPersonControllerObject->IsAutonomousSoConnect();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->IsAutonomousSoConnect();
            AssignConnectInputEvents();
            if (IsNetEntityRoleAuthority())
                m_firstPersonControllerObject->m_isHost = true;
            else
                m_firstPersonControllerObject->m_isAutonomousClient = true;
        }
        else if (IsNetEntityRoleAuthority())
        {
            m_isAuthority = true;
        }
    }

    void NetworkFPCController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkFPCControllerRequestBus::Handler::BusDisconnect();
        InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void NetworkFPCController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void NetworkFPCController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
    }

    void NetworkFPCController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkFPCService"));
    }

    void NetworkFPCController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkFPCService"));
        incompatible.push_back(AZ_CRC_CE("InputService"));
    }

    void NetworkFPCController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        // Assign input values
        playerInput->m_forward = m_forwardValue;
        playerInput->m_back = m_backValue;
        playerInput->m_left = m_leftValue;
        playerInput->m_right = m_rightValue;
        playerInput->m_yaw = m_yawValue / (deltaTime / m_firstPersonControllerObject->m_prevDeltaTime);
        playerInput->m_pitch = m_pitchValue / (deltaTime / m_firstPersonControllerObject->m_prevDeltaTime);
        playerInput->m_sprint = m_sprintValue;
        playerInput->m_crouch = m_crouchValue;
        playerInput->m_jump = m_jumpValue;

        m_yawValue = 0.0f;
        m_pitchValue = 0.0f;
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        // Disconnect from various buses when the NetworkFPCController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_firstPersonControllerObject->NotAutonomousSoDisconnect();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_firstPersonControllerObject->m_isServer = true;
            m_autonomousNotDetermined = false;
        }

        NetworkFPCControllerNotificationBus::Broadcast(&NetworkFPCControllerNotificationBus::Events::OnNetworkTick, deltaTime);

        const auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        // Assign the First Person Controller's inputs from the input prediction
        m_firstPersonControllerObject->m_forwardValue = playerInput->m_forward;
        m_firstPersonControllerObject->m_backValue = playerInput->m_back;
        m_firstPersonControllerObject->m_leftValue = playerInput->m_left;
        m_firstPersonControllerObject->m_rightValue = playerInput->m_right;
        m_firstPersonControllerObject->m_yawValue = playerInput->m_yaw;
        m_firstPersonControllerObject->m_pitchValue = playerInput->m_pitch;
        m_firstPersonControllerObject->m_sprintValue = playerInput->m_sprint;
        m_firstPersonControllerObject->m_crouchValue = playerInput->m_crouch;
        m_firstPersonControllerObject->m_jumpValue = playerInput->m_jump;

        if (playerInput->m_sprint != 0.f &&
            (m_firstPersonControllerObject->m_grounded || m_firstPersonControllerObject->m_sprintPrevValue == 0.f))
        {
            m_firstPersonControllerObject->m_sprintEffectiveValue = playerInput->m_sprint;
            m_firstPersonControllerObject->m_sprintAccelValue = playerInput->m_sprint * m_firstPersonControllerObject->m_sprintAccelScale;
        }
        else
        {
            m_firstPersonControllerObject->m_sprintEffectiveValue = 0.f;
            m_firstPersonControllerObject->m_sprintAccelValue = 0.f;
        }

        GetNetworkCharacterComponentController()->TryMoveWithVelocity(
            m_firstPersonControllerObject->m_prevTargetVelocity, (deltaTime + m_prevDeltaTime) / 2.f);
        m_prevDeltaTime = deltaTime;

        // AZ_Printf("NetworkFPC", "Forward: %f", playerInput->m_forward);
        // AZ_Printf("NetworkFPC", "Back: %f", playerInput->m_back);
        // AZ_Printf("NetworkFPC", "Left: %f", playerInput->m_left);
        // AZ_Printf("NetworkFPC", "Right: %f", playerInput->m_right);
        // AZ_Printf("NetworkFPC", "Yaw: %f", playerInput->m_yaw);
        // AZ_Printf("NetworkFPC", "Pitch: %f", playerInput->m_pitch);
        // AZ_Printf("NetworkFPC", "Sprint: %f", playerInput->m_sprint);
        // AZ_Printf("NetworkFPC", "Crouch: %f", playerInput->m_crouch);
        // AZ_Printf("NetworkFPC", "Jump: %f", playerInput->m_jump);
    }

    // Event Notification methods for use in scripts
    void NetworkFPCController::OnNetworkTick([[maybe_unused]] const float& deltaTime)
    {
    }

    void NetworkFPCController::OnEnableNetworkFPCChanged(const bool& enable)
    {
        m_disabled = !enable;
        m_firstPersonControllerObject->m_networkFPCEnabled = enable;
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = enable;
        if (!m_disabled)
        {
            m_firstPersonControllerObject->NetworkFPCEnabledIgnoreInputs();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->NetworkFPCEnabledIgnoreInputs();
            AssignConnectInputEvents();
        }
        else
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
            m_firstPersonControllerObject->AssignConnectInputEvents();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->AssignConnectInputEvents();
        }
    }

    // Request Bus getter and setter methods for use in scripts
    void NetworkFPCController::TryAddVelocityForNetworkTick(const AZ::Vector3& tryVelocity, const float& deltaTime)
    {
        GetNetworkCharacterComponentController()->TryMoveWithVelocity(tryVelocity, deltaTime);
    }
    bool NetworkFPCController::GetIsNetEntityAutonomous() const
    {
        return IsNetEntityRoleAutonomous();
    }
    bool NetworkFPCController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkFPCController::SetEnabled(const bool& new_enabled)
    {
        m_disabled = !new_enabled;
        m_firstPersonControllerObject->m_networkFPCEnabled = new_enabled;
        if (m_firstPersonExtrasObject != nullptr)
            m_firstPersonExtrasObject->m_networkFPCEnabled = new_enabled;
        if (!m_disabled)
        {
            m_firstPersonControllerObject->NetworkFPCEnabledIgnoreInputs();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->NetworkFPCEnabledIgnoreInputs();
            AssignConnectInputEvents();
        }
        else
        {
            InputEventNotificationBus::MultiHandler::BusDisconnect();
            m_firstPersonControllerObject->AssignConnectInputEvents();
            if (m_firstPersonExtrasObject != nullptr)
                m_firstPersonExtrasObject->AssignConnectInputEvents();
        }
    }
} // namespace FirstPersonController
