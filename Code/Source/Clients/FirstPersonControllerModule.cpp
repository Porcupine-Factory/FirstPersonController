/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FirstPersonControllerSystemComponent.h"
#include <FirstPersonController/FirstPersonControllerTypeIds.h>
#include <FirstPersonControllerModuleInterface.h>

namespace FirstPersonController
{
    class FirstPersonControllerModule : public FirstPersonControllerModuleInterface
    {
    public:
        AZ_RTTI(FirstPersonControllerModule, FirstPersonControllerModuleTypeId, FirstPersonControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(FirstPersonControllerModule, AZ::SystemAllocator);
    };
} // namespace FirstPersonController

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), FirstPersonController::FirstPersonControllerModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_FirstPersonController, FirstPersonController::FirstPersonControllerModule)
#endif
