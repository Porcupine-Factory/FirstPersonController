
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Clients/FirstPersonControllerSystemComponent.h>
#include <Clients/FirstPersonControllerComponent.h>

namespace FirstPersonController
{
    class FirstPersonControllerModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(FirstPersonControllerModuleInterface, "{2D84A6BC-BAE1-4557-9CE2-7EBDCF692301}", AZ::Module);
        AZ_CLASS_ALLOCATOR(FirstPersonControllerModuleInterface, AZ::SystemAllocator, 0);

        FirstPersonControllerModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                FirstPersonControllerSystemComponent::CreateDescriptor(),
                FirstPersonControllerComponent::CreateDescriptor()
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<FirstPersonControllerSystemComponent>(),
            };
        }
    };
}// namespace FirstPersonController
