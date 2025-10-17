/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FirstPersonControllerEditorSystemComponent.h"
#include <AzCore/Serialization/SerializeContext.h>

namespace FirstPersonController
{
    void FirstPersonControllerEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FirstPersonControllerEditorSystemComponent, FirstPersonControllerSystemComponent>()->Version(0);
        }
    }

    FirstPersonControllerEditorSystemComponent::FirstPersonControllerEditorSystemComponent() = default;

    FirstPersonControllerEditorSystemComponent::~FirstPersonControllerEditorSystemComponent() = default;

    void FirstPersonControllerEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("FirstPersonControllerEditorService"));
    }

    void FirstPersonControllerEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("FirstPersonControllerEditorService"));
    }

    void FirstPersonControllerEditorSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void FirstPersonControllerEditorSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void FirstPersonControllerEditorSystemComponent::Activate()
    {
        FirstPersonControllerSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void FirstPersonControllerEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        FirstPersonControllerSystemComponent::Deactivate();
    }

} // namespace FirstPersonController
