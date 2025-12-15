/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FirstPersonControllerModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <FirstPersonController/FirstPersonControllerTypeIds.h>

#include <Clients/FirstPersonControllerSystemComponent.h>

namespace FirstPersonController
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        FirstPersonControllerModuleInterface, "FirstPersonControllerModuleInterface", FirstPersonControllerModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(FirstPersonControllerModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(FirstPersonControllerModuleInterface, AZ::SystemAllocator);

    FirstPersonControllerModuleInterface::FirstPersonControllerModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(
            m_descriptors.end(),
            { FirstPersonControllerSystemComponent::CreateDescriptor(),
              FirstPersonControllerComponent::CreateDescriptor(),
              FirstPersonExtrasComponent::CreateDescriptor(),
              CameraCoupledChildComponent::CreateDescriptor() });

        //< Register multiplayer components
        CreateComponentDescriptors(m_descriptors);
    }

    AZ::ComponentTypeList FirstPersonControllerModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<FirstPersonControllerSystemComponent>(),
        };
    }
} // namespace FirstPersonController
