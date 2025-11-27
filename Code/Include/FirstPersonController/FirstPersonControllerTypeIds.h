
#pragma once

namespace FirstPersonController
{
    // System Component TypeIds
    inline constexpr const char* FirstPersonControllerSystemComponentTypeId = "{722F28F9-359E-4FA7-B6A2-0FEA0485C8D3}";
    inline constexpr const char* FirstPersonControllerEditorSystemComponentTypeId = "{DEA3CCF3-DBBF-486F-A03A-C3B56A6684AC}";

    // Module derived classes TypeIds
    inline constexpr const char* FirstPersonControllerModuleInterfaceTypeId = "{0C3E7E89-9134-4F85-9357-7839BE198342}";
    inline constexpr const char* FirstPersonControllerModuleTypeId = "{17657A7A-BFBD-448A-B1B5-5B31A34A00D9}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* FirstPersonControllerEditorModuleTypeId = FirstPersonControllerModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* FirstPersonControllerRequestsTypeId = "{5CFAB79E-4680-4ABE-8E49-3F6FA7A91698}";
} // namespace FirstPersonController
