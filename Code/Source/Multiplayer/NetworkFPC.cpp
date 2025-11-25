#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    NetworkFPCController::NetworkFPCController(NetworkFPC& parent)
        : NetworkFPCControllerBase(parent)
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

        if (*inputId == m_sprintEventId)
        {
            m_sprintValue = value;
        }

        for (auto& it_event : m_controlMap)
        {
            if (*inputId == *(it_event.first) && !(*(it_event.first) == m_sprintEventId))
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
            m_yawValue = value;
        }
        else if (*inputId == m_rotatePitchEventId)
        {
            m_pitchValue = value;
        }
        // Repeatedly update the sprint value since we are setting it to 1 under certain movement conditions
        else if (*inputId == m_sprintEventId)
        {
            m_sprintValue = value;
        }
    }

    void NetworkFPCController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        if (IsNetEntityRoleAutonomous())
            AssignConnectInputEvents();

        NetworkFPCControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Get access to the FirstPersonControllerComponent and FirstPersonExtrasComponent objects and their members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_firstPersonControllerObject = entity->FindComponent<FirstPersonControllerComponent>();
        m_firstPersonExtrasObject = entity->FindComponent<FirstPersonExtrasComponent>();
    }

    void NetworkFPCController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkFPCControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkFPCController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        required.push_back(AZ_CRC_CE("FirstPersonControllerService"));
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
        playerInput->m_yaw = m_yawValue;
        playerInput->m_pitch = m_pitchValue;
        playerInput->m_sprint = m_sprintValue;
        playerInput->m_crouch = m_crouchValue;
        playerInput->m_jump = m_jumpValue;

        m_yawValue = 0.0f;
        m_pitchValue = 0.0f;
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (!GetEnableNetworkFPC())
            return;

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

    // Request Bus getter and setter methods for use in scripts
    void NetworkFPCController::TryAddVelocityForNetworkTick(const AZ::Vector3& tryVelocity, const float& deltaTime)
    {
        GetNetworkCharacterComponentController()->TryMoveWithVelocity(tryVelocity, deltaTime);
    }
    bool NetworkFPCController::GetIsNetEntityAutonomous() const
    {
        return IsNetEntityRoleAutonomous();
    }
} // namespace FirstPersonController
