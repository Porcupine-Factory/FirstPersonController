/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <FirstPersonController/FirstPersonControllerBus.h>
#include <FirstPersonController/FirstPersonExtrasBus.h>
#include <FirstPersonController/NetworkFPCBus.h>

namespace FirstPersonController
{
    class FirstPersonControllerSystemComponent
        : public AZ::Component
        , protected FirstPersonControllerRequestBus::Handler
        , protected FirstPersonExtrasRequestBus::Handler
        , protected NetworkFPCRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(FirstPersonControllerSystemComponent, "{736A6201-CF72-489D-BA29-89F01B14914F}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        FirstPersonControllerSystemComponent();
        ~FirstPersonControllerSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // FirstPersonControllerRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////
    };

} // namespace FirstPersonController
