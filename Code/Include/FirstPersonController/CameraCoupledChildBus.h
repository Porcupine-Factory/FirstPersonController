/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FirstPersonController
{
    class CameraCoupledChildRequests
    {
    public:
        AZ_RTTI(CameraCoupledChildRequests, "{837606A2-5573-45DB-828A-7BCC245DFEE7}");
        virtual ~CameraCoupledChildRequests() = default;
        // Put your public methods here
    };

    class CameraCoupledChildBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using CameraCoupledChildRequestBus = AZ::EBus<CameraCoupledChildRequests, CameraCoupledChildBusTraits>;
    using CameraCoupledChildInterface = AZ::Interface<CameraCoupledChildRequests>;

} // namespace FirstPersonController
