/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <AzCore/Memory/Memory_fwd.h>
#include <AzCore/Module/Module.h>
#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <Clients/FirstPersonControllerComponent.h>
#include <Clients/FirstPersonControllerSystemComponent.h>
#include <Clients/FirstPersonExtrasComponent.h>
#include <Source/AutoGen/AutoComponentTypes.h>

namespace FirstPersonController
{
    class FirstPersonControllerModuleInterface : public AZ::Module
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(FirstPersonControllerModuleInterface)
        AZ_RTTI_NO_TYPE_INFO_DECL()
        AZ_CLASS_ALLOCATOR_DECL

        FirstPersonControllerModuleInterface();

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override;
    };
} // namespace FirstPersonController
