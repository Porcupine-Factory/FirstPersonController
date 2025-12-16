/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class CameraCoupledChildComponentRequests : public AZ::ComponentBus
    {
    public:
        ~CameraCoupledChildComponentRequests() override = default;

        virtual bool GetEnableCameraCoupledChild() const = 0;
        virtual void SetEnableCameraCoupledChild(const bool&) = 0;
        virtual float GetInitialZOffset() const = 0;
        virtual void SetInitialZOffset(const float&) = 0;
    };

    using CameraCoupledChildComponentRequestBus = AZ::EBus<CameraCoupledChildComponentRequests>;

    class CameraCoupledChildComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnPlaceholder() = 0;
    };

    using CameraCoupledChildComponentNotificationBus = AZ::EBus<CameraCoupledChildComponentNotifications>;

    class CameraCoupledChildComponentNotificationHandler
        : public CameraCoupledChildComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            CameraCoupledChildComponentNotificationHandler, "{766eea87-14cd-4c61-bfbc-a40c92e7554a}", AZ::SystemAllocator, OnPlaceholder);

        void OnPlaceholder() override
        {
            Call(FN_OnPlaceholder);
        }
    };
} // namespace FirstPersonController
