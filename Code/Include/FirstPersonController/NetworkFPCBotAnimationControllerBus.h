/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class NetworkFPCBotAnimationControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkFPCBotAnimationControllerRequests() override = default;
    };

    using NetworkFPCBotAnimationControllerRequestBus = AZ::EBus<NetworkFPCBotAnimationControllerRequests>;

    class NetworkFPCBotAnimationControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnPlaceholder() = 0;
    };

    using NetworkFPCBotAnimationControllerNotificationBus = AZ::EBus<NetworkFPCBotAnimationControllerNotifications>;

    class NetworkFPCBotAnimationControllerNotificationHandler
        : public NetworkFPCBotAnimationControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkFPCBotAnimationControllerNotificationHandler,
            "{b7632119-cd76-4dbe-847c-052b29466cb8}",
            AZ::SystemAllocator,
            OnPlaceholder);

        void OnPlaceholder() override
        {
            Call(FN_OnPlaceholder);
        }
    };
} // namespace FirstPersonController
