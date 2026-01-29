/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FirstPersonController
{
    class NetworkFPCBotAnimationRequests
    {
    public:
        AZ_RTTI(NetworkFPCBotAnimationRequests, "{AC8DE3CA-B4F8-433B-94BC-07D91D23D7AE}");
        virtual ~NetworkFPCBotAnimationRequests() = default;
        // Put your public methods here
    };

    class NetworkFPCBotAnimationBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkFPCBotAnimationRequestBus = AZ::EBus<NetworkFPCBotAnimationRequests, NetworkFPCBotAnimationBusTraits>;
    using NetworkFPCBotAnimationInterface = AZ::Interface<NetworkFPCBotAnimationRequests>;

} // namespace FirstPersonController
