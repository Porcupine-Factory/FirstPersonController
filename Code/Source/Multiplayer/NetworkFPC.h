#pragma once
#include <FirstPersonController/NetworkFPCControllerBus.h>

#include <Source/AutoGen/NetworkFPC.AutoComponent.h>

#include <Clients/FirstPersonControllerComponent.h>

namespace FirstPersonController
{

    class NetworkFPCController
        : public NetworkFPCControllerBase
        , public NetworkFPCControllerRequestBus::Handler
    {
    public:
        explicit NetworkFPCController(NetworkFPC& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

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
        void TryMoveWithVelocity(const AZ::Vector3& tryVelocity, const float& deltaTime) const override;

    protected:
        // NetworkFPCControllerNotificationBus
        void OnNetworkTick(const float& deltaTime);

        friend class FirstPersonControllerComponent;
        friend class FirstPersonExtrasComponent;
    };
} // namespace FirstPersonController
