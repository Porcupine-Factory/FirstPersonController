
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class NetworkFPCControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkFPCControllerRequests() override = default;

        virtual void TryAddVelocityForNetworkTick(const AZ::Vector3&, const float&) = 0;
        virtual bool GetNetworkFPCEnabled() const = 0;
        virtual void SetNetworkFPCEnabled(const bool&) = 0;
    };

    using NetworkFPCControllerRequestBus = AZ::EBus<NetworkFPCControllerRequests>;

    class NetworkFPCControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTick(const float&) = 0;
    };

    using NetworkFPCControllerNotificationBus = AZ::EBus<NetworkFPCControllerNotifications>;

    class NetworkFPCControllerNotificationHandler
        : public NetworkFPCControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkFPCControllerNotificationHandler, "{4f610d12-82bc-4e01-a792-7730beb321d0}", AZ::SystemAllocator, OnNetworkTick);

        void OnNetworkTick([[maybe_unused]] const float& deltaTime) override
        {
            Call(FN_OnNetworkTick);
        }
    };
} // namespace FirstPersonController
