#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <Multiplayer/Components/NetworkCharacterComponent.h>

namespace FirstPersonController
{
    NetworkFPCController::NetworkFPCController(NetworkFPC& parent)
        : NetworkFPCControllerBase(parent)
    {
    }

    void NetworkFPCController::Reflect(AZ::ReflectContext* rc)
    {
        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<NetworkFPCControllerNotificationBus>("NetworkFPCControllerNotificationBus")
                ->Handler<NetworkFPCControllerNotificationHandler>();

            bc->EBus<NetworkFPCControllerRequestBus>("NetworkFPCControllerRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "network controller")
                ->Attribute(AZ::Script::Attributes::Category, "Network FPC")
                ->Event("Try Add Velocity For Network Tick", &NetworkFPCControllerRequests::TryAddVelocityForNetworkTick)
                ->Event("Get NetworkFPC Enabled", &NetworkFPCControllerRequests::GetNetworkFPCEnabled)
                ->Event("Set NetworkFPC Enabled", &NetworkFPCControllerRequests::SetNetworkFPCEnabled);

            bc->Class<FirstPersonControllerComponent>()->RequestBus("NetworkFPCControllerRequestBus");
        }
    }

    void NetworkFPCController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
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
        // Retrieve the FirstPersonControllerComponent from the entity
        if (m_firstPersonControllerObject)
        {
            auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

            // Assign input values from the FirstPersonControllerComponent
            playerInput->m_forward = m_firstPersonControllerObject->m_forwardValue;
            playerInput->m_back = m_firstPersonControllerObject->m_backValue;
            playerInput->m_left = m_firstPersonControllerObject->m_leftValue;
            playerInput->m_right = m_firstPersonControllerObject->m_rightValue;
            playerInput->m_yaw = m_firstPersonControllerObject->m_yawValue;
            playerInput->m_pitch = m_firstPersonControllerObject->m_pitchValue;
            playerInput->m_jump = m_firstPersonControllerObject->m_jumpValue;
            playerInput->m_crouch = m_firstPersonControllerObject->m_crouchValue;
        }
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (!m_enable)
            return;

        NetworkFPCControllerNotificationBus::Broadcast(&NetworkFPCControllerNotificationBus::Events::OnNetworkTick, deltaTime);

        // const auto* playerInput = input.FindComponentInput<NetworkFPCNetworkInput>();

        // AZ_Printf("NetworkFPC", "Forward: %f", playerInput->m_forward);
        // AZ_Printf("NetworkFPC", "Back: %f", playerInput->m_back);
        // AZ_Printf("NetworkFPC", "Left: %f", playerInput->m_left);
        // AZ_Printf("NetworkFPC", "Right: %f", playerInput->m_right);
        // AZ_Printf("NetworkFPC", "Yaw: %f", playerInput->m_yaw);
        // AZ_Printf("NetworkFPC", "Pitch: %f", playerInput->m_pitch);
        // AZ_Printf("NetworkFPC", "Jump: %f", playerInput->m_jump);
        // AZ_Printf("NetworkFPC", "Crouch: %f", playerInput->m_crouch);
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
    bool NetworkFPCController::GetNetworkFPCEnabled() const
    {
        return m_enable;
    }
    void NetworkFPCController::SetNetworkFPCEnabled(const bool& new_enable)
    {
        m_enable = new_enable;
    }
} // namespace FirstPersonController
