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

        virtual void TryAddVelocityForNetworkTick(const AZ::Vector3&, const float) = 0;
        virtual bool GetAllowAllMovementInputs() const = 0;
        virtual void SetAllowAllMovementInputs(const bool) = 0;
        virtual bool GetAllowRotationInputs() const = 0;
        virtual void SetAllowRotationInputs(const bool) = 0;
        virtual AZ::TimeMs GetHostTimeMs() const = 0;
        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool) = 0;
        virtual bool GetIsNetEntityRoleAuthority() const = 0;
        virtual float GetActionInputValue() const = 0;
        virtual float GetAttackInputValue() const = 0;
        virtual float GetBlockInputValue() const = 0;
        virtual float GetReloadInputValue() const = 0;
        virtual float GetNextWeaponInputValue() const = 0;
        virtual float GetPrevWeaponInputValue() const = 0;
    };

    using NetworkFPCControllerRequestBus = AZ::EBus<NetworkFPCControllerRequests>;

    class NetworkFPCControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float, const bool, const AZ::EntityId&) {};
        virtual void OnNetworkTickFinish(const float, const bool, const AZ::EntityId&) {};
        virtual void OnAutonomousClientActivated(const AZ::EntityId&) {};
        virtual void OnHostActivated(const AZ::EntityId&) {};
        virtual void OnNonAutonomousClientActivated(const AZ::EntityId&) {};
        virtual void OnActionPressed(const float) {};
        virtual void OnActionReleased(const float) {};
        virtual void OnAttackPressed(const float) {};
        virtual void OnAttackReleased(const float) {};
        virtual void OnBlockPressed(const float) {};
        virtual void OnBlockReleased(const float) {};
        virtual void OnReloadPressed(const float) {};
        virtual void OnReloadReleased(const float) {};
        virtual void OnNextWeaponPressed(const float) {};
        virtual void OnNextWeaponReleased(const float) {};
        virtual void OnPrevWeaponPressed(const float) {};
        virtual void OnPrevWeaponReleased(const float) {};
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
            OnHostActivated,
            OnNonAutonomousClientActivated,
            OnActionPressed,
            OnActionReleased,
            OnAttackPressed,
            OnAttackReleased,
            OnBlockPressed,
            OnBlockReleased,
            OnReloadPressed,
            OnReloadReleased,
            OnNextWeaponPressed,
            OnNextWeaponReleased,
            OnPrevWeaponPressed,
            OnPrevWeaponReleased);

        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId) override
        {
            Call(FN_OnNetworkTickStart, deltaTime, server, entityId);
        }
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId) override
        {
            Call(FN_OnNetworkTickFinish, deltaTime, server, entityId);
        }
        void OnAutonomousClientActivated(const AZ::EntityId& entityId) override
        {
            Call(FN_OnAutonomousClientActivated, entityId);
        }
        void OnHostActivated(const AZ::EntityId& entityId) override
        {
            Call(FN_OnHostActivated, entityId);
        }
        void OnNonAutonomousClientActivated(const AZ::EntityId& entityId) override
        {
            Call(FN_OnNonAutonomousClientActivated, entityId);
        }
        void OnActionPressed(const float actionValue) override
        {
            Call(FN_OnActionPressed, actionValue);
        }
        void OnActionReleased(const float actionValue) override
        {
            Call(FN_OnActionReleased, actionValue);
        }
        void OnAttackPressed(const float attackValue) override
        {
            Call(FN_OnAttackPressed, attackValue);
        }
        void OnAttackReleased(const float attackValue) override
        {
            Call(FN_OnAttackReleased, attackValue);
        }
        void OnBlockPressed(const float blockValue) override
        {
            Call(FN_OnBlockPressed, blockValue);
        }
        void OnBlockReleased(const float blockValue) override
        {
            Call(FN_OnBlockReleased, blockValue);
        }
        void OnReloadPressed(const float reloadValue) override
        {
            Call(FN_OnReloadPressed, reloadValue);
        }
        void OnReloadReleased(const float reloadValue) override
        {
            Call(FN_OnReloadReleased, reloadValue);
        }
        void OnNextWeaponPressed(const float nextWeaponValue) override
        {
            Call(FN_OnNextWeaponPressed, nextWeaponValue);
        }
        void OnNextWeaponReleased(const float nextWeaponValue) override
        {
            Call(FN_OnNextWeaponReleased, nextWeaponValue);
        }
        void OnPrevWeaponPressed(const float prevWeaponValue) override
        {
            Call(FN_OnPrevWeaponPressed, prevWeaponValue);
        }
        void OnPrevWeaponReleased(const float prevWeaponValue) override
        {
            Call(FN_OnPrevWeaponReleased, prevWeaponValue);
        }
    };
} // namespace FirstPersonController
