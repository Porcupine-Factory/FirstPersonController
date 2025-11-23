/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FirstPersonController
{
    class NetworkFPCRequests
    {
    public:
        AZ_RTTI(NetworkFPCRequests, "{462A6CB2-771A-4C9C-9CBA-336EF989E955}");
        virtual ~NetworkFPCRequests() = default;
        // Put your public methods here
    };

    class NetworkFPCBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkFPCRequestBus = AZ::EBus<NetworkFPCRequests, NetworkFPCBusTraits>;
    using NetworkFPCInterface = AZ::Interface<NetworkFPCRequests>;

} // namespace FirstPersonController
