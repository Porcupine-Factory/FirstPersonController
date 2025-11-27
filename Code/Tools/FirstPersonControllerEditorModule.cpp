
#include "FirstPersonControllerEditorSystemComponent.h"
#include <FirstPersonController/FirstPersonControllerTypeIds.h>
#include <FirstPersonControllerModuleInterface.h>

namespace FirstPersonController
{
    class FirstPersonControllerEditorModule : public FirstPersonControllerModuleInterface
    {
    public:
        AZ_RTTI(FirstPersonControllerEditorModule, FirstPersonControllerEditorModuleTypeId, FirstPersonControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(FirstPersonControllerEditorModule, AZ::SystemAllocator);

        FirstPersonControllerEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    FirstPersonControllerEditorSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<FirstPersonControllerEditorSystemComponent>(),
            };
        }
    };
} // namespace FirstPersonController

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), FirstPersonController::FirstPersonControllerEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_FirstPersonController_Editor, FirstPersonController::FirstPersonControllerEditorModule)
#endif
