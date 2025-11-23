#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>

namespace FirstPersonController
{
    NetworkFPCController::NetworkFPCController(NetworkFPC& parent)
        : NetworkFPCControllerBase(parent)
    {
    }

    void NetworkFPCController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkFPCController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
    }

    void NetworkFPCController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkFPCService"));
    }

    void NetworkFPCController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        // Retrieve the FirstPersonControllerComponent from the entity
        FirstPersonControllerComponent* firstPersonController = GetParent().GetEntity()->FindComponent<FirstPersonControllerComponent>();
        if (firstPersonController)
        {
            auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

            // Assign input values from the FirstPersonControllerComponent
            playerInput->m_forward = firstPersonController->m_forwardValue;
            playerInput->m_back = firstPersonController->m_backValue;
            playerInput->m_left = firstPersonController->m_leftValue;
            playerInput->m_right = firstPersonController->m_rightValue;
            playerInput->m_yaw = firstPersonController->m_yawValue;
            playerInput->m_pitch = firstPersonController->m_pitchValue;
            playerInput->m_jump = firstPersonController->m_jumpValue;
            playerInput->m_crouch = firstPersonController->m_crouchValue;

            // Reset accumulated deltas for yaw and pitch
            firstPersonController->m_yawValue = 0.0f;
            firstPersonController->m_pitchValue = 0.0f;
        }
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
        NetworkFPCControllerNotificationBus::Broadcast(&NetworkFPCControllerNotificationBus::Events::OnNetworkTick, deltaTime);

        const auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        AZ_Printf("NetworkFPC", "Forward: %f", playerInput->m_forward);
        AZ_Printf("NetworkFPC", "Back: %f", playerInput->m_back);
        AZ_Printf("NetworkFPC", "Left: %f", playerInput->m_left);
        AZ_Printf("NetworkFPC", "Right: %f", playerInput->m_right);
        AZ_Printf("NetworkFPC", "Yaw: %f", playerInput->m_yaw);
        AZ_Printf("NetworkFPC", "Pitch: %f", playerInput->m_pitch);
        AZ_Printf("NetworkFPC", "Jump: %f", playerInput->m_jump);
        AZ_Printf("NetworkFPC", "Crouch: %f", playerInput->m_crouch);
    }

    // Event Notification methods for use in scripts
    void NetworkFPCController::OnNetworkTick([[maybe_unused]] const float& deltaTime)
    {
    }

    // Request Bus getter and setter methods for use in scripts
    void NetworkFPCController::TryMoveWithVelocity(const AZ::Vector3& tryVelocity, const float& deltaTime) const
    {
        // GetController()->TryMoveWithVelocity(tryVelocity, deltaTime);
    }
} // namespace FirstPersonController
