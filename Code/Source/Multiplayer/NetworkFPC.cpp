#include <Multiplayer/NetworkFPC.h>

#include <AzCore/Serialization/SerializeContext.h>

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

    void NetworkFPCController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkFPCController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }
} // namespace FirstPersonController
