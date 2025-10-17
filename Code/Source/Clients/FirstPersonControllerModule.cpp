/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FirstPersonControllerSystemComponent.h"
#include <FirstPersonControllerModuleInterface.h>

namespace FirstPersonController
{
    class FirstPersonControllerModule : public FirstPersonControllerModuleInterface
    {
    public:
        AZ_RTTI(FirstPersonControllerModule, "{D78F270B-260D-44F0-8ED0-89A2D9D0D8FD}", FirstPersonControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(FirstPersonControllerModule, AZ::SystemAllocator, 0);
    };
} // namespace FirstPersonController

AZ_DECLARE_MODULE_CLASS(Gem_FirstPersonController, FirstPersonController::FirstPersonControllerModule)
