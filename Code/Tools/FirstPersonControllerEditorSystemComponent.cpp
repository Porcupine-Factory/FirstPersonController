
#include "FirstPersonControllerEditorSystemComponent.h"
#include <AzCore/Serialization/SerializeContext.h>

#include <FirstPersonController/FirstPersonControllerTypeIds.h>

namespace FirstPersonController
{
    AZ_COMPONENT_IMPL(
        FirstPersonControllerEditorSystemComponent,
        "FirstPersonControllerEditorSystemComponent",
        FirstPersonControllerEditorSystemComponentTypeId,
        BaseSystemComponent);

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
