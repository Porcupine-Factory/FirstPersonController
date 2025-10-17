/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FirstPersonController
{
    class FirstPersonControllerRequests
    {
    public:
        AZ_RTTI(FirstPersonControllerRequests, "{2880DB3D-3966-4C87-8777-BC9028E3F48D}");
        virtual ~FirstPersonControllerRequests() = default;
        // Put your public methods here
    };

    class FirstPersonControllerBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using FirstPersonControllerRequestBus = AZ::EBus<FirstPersonControllerRequests, FirstPersonControllerBusTraits>;
    using FirstPersonControllerInterface = AZ::Interface<FirstPersonControllerRequests>;

} // namespace FirstPersonController
