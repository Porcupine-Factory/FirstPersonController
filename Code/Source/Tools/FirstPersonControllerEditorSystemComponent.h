/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/FirstPersonControllerSystemComponent.h>

namespace FirstPersonController
{
    /// System component for FirstPersonController editor
    class FirstPersonControllerEditorSystemComponent
        : public FirstPersonControllerSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = FirstPersonControllerSystemComponent;

    public:
        AZ_COMPONENT(FirstPersonControllerEditorSystemComponent, "{AF02CA9C-C8AC-4F6C-84EC-111722575DB1}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        FirstPersonControllerEditorSystemComponent();
        ~FirstPersonControllerEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace FirstPersonController
