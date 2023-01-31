/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <FirstPersonControllerModuleInterface.h>
#include "FirstPersonControllerEditorSystemComponent.h"

namespace FirstPersonController
{
    class FirstPersonControllerEditorModule
        : public FirstPersonControllerModuleInterface
    {
    public:
        AZ_RTTI(FirstPersonControllerEditorModule, "{D78F270B-260D-44F0-8ED0-89A2D9D0D8FD}", FirstPersonControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(FirstPersonControllerEditorModule, AZ::SystemAllocator, 0);

        FirstPersonControllerEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                FirstPersonControllerEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<FirstPersonControllerEditorSystemComponent>(),
            };
        }
    };
}// namespace FirstPersonController

AZ_DECLARE_MODULE_CLASS(Gem_FirstPersonController, FirstPersonController::FirstPersonControllerEditorModule)
