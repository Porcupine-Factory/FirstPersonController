/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <Multiplayer/NetworkTime/INetworkTime.h>

namespace FirstPersonController
{
    class NetworkFPCControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkFPCControllerRequests() override = default;

        virtual void TryAddVelocityForNetworkTick(const AZ::Vector3&, const float&) = 0;
        virtual bool GetAllowAllMovementInputs() const = 0;
        virtual void SetAllowAllMovementInputs(const bool&) = 0;
        virtual bool GetAllowRotationInputs() const = 0;
        virtual void SetAllowRotationInputs(const bool&) = 0;
        virtual AZ::TimeMs GetHostTimeMs() const = 0;
        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool&) = 0;
        virtual bool GetIsNetEntityRoleAuthority() const = 0;
    };

    using NetworkFPCControllerRequestBus = AZ::EBus<NetworkFPCControllerRequests>;

    class NetworkFPCControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float&, const bool&, const AZ::EntityId&) = 0;
        virtual void OnNetworkTickFinish(const float&, const bool&, const AZ::EntityId&) = 0;
        virtual void OnAutonomousClientActivated(const AZ::EntityId&) = 0;
        virtual void OnHostActivated(const AZ::EntityId&) = 0;
    };

    using NetworkFPCControllerNotificationBus = AZ::EBus<NetworkFPCControllerNotifications>;

    class NetworkFPCControllerNotificationHandler
        : public NetworkFPCControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkFPCControllerNotificationHandler,
            "{4f610d12-82bc-4e01-a792-7730beb321d0}",
            AZ::SystemAllocator,
            OnNetworkTickStart,
            OnNetworkTickFinish,
            OnAutonomousClientActivated,
            OnHostActivated);

        void OnNetworkTickStart(
            [[maybe_unused]] const float& deltaTime,
            [[maybe_unused]] const bool& server,
            [[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnNetworkTickStart);
        }
        void OnNetworkTickFinish(
            [[maybe_unused]] const float& deltaTime,
            [[maybe_unused]] const bool& server,
            [[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnNetworkTickFinish);
        }
        void OnAutonomousClientActivated([[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnAutonomousClientActivated);
        }
        void OnHostActivated([[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnHostActivated);
        }
    };
} // namespace FirstPersonController
