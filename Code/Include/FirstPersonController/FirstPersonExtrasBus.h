/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FirstPersonController
{
    class FirstPersonExtrasRequests
    {
    public:
        AZ_RTTI(FirstPersonExtrasRequests, "{E6F98502-3291-46C7-87AD-91BCE83A68BF}");
        virtual ~FirstPersonExtrasRequests() = default;
        // Put your public methods here
    };

    class FirstPersonExtrasBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using FirstPersonExtrasRequestBus = AZ::EBus<FirstPersonExtrasRequests, FirstPersonExtrasBusTraits>;
    using FirstPersonExtrasInterface = AZ::Interface<FirstPersonExtrasRequests>;

} // namespace FirstPersonController
