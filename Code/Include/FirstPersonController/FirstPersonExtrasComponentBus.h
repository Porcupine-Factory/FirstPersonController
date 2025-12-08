
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace FirstPersonController
{
    class FirstPersonExtrasComponentRequests : public AZ::ComponentBus
    {
    public:
        ~FirstPersonExtrasComponentRequests() override = default;

        virtual float GetJumpPressedInAirQueueTimeThreshold() const = 0;
        virtual void SetJumpPressedInAirQueueTimeThreshold(const float&) = 0;
        virtual bool GetEnableHeadbob() const = 0;
        virtual AZ::EntityId GetHeadbobEntityId() const = 0;
        virtual void SetHeadbobEntityId(const AZ::EntityId&) = 0;
        virtual AZ::Vector3 GetCameraTranslationWithoutHeadbob() const = 0;
        virtual AZ::Vector3 GetPreviousOffset() const = 0;
        virtual void NetworkFPCEnabledIgnoreInputs() = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using FirstPersonExtrasComponentRequestBus = AZ::EBus<FirstPersonExtrasComponentRequests>;

    class FirstPersonExtrasComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnJumpFromQueue() = 0;
    };

    using FirstPersonExtrasComponentNotificationBus = AZ::EBus<FirstPersonExtrasComponentNotifications>;

    class FirstPersonExtrasComponentNotificationHandler
        : public FirstPersonExtrasComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            FirstPersonExtrasComponentNotificationHandler, "{3fafe1bc-50bb-42ba-bd1f-f20df1706517}", AZ::SystemAllocator, OnJumpFromQueue);

        void OnJumpFromQueue() override
        {
            Call(FN_OnJumpFromQueue);
        }
    };
} // namespace FirstPersonController
