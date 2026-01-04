/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FirstPersonControllerSystemComponent.h"

#include <FirstPersonController/FirstPersonControllerTypeIds.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#if __has_include(<Source/AutoGen/AutoComponentTypes.h>)
#include <Source/AutoGen/AutoComponentTypes.h>
#endif

namespace FirstPersonController
{
    AZ_COMPONENT_IMPL(
        FirstPersonControllerSystemComponent, "FirstPersonControllerSystemComponent", FirstPersonControllerSystemComponentTypeId);

    void FirstPersonControllerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FirstPersonControllerSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<FirstPersonControllerSystemComponent>(
                      "FirstPersonController", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);

                ec->Class<FirstPersonControllerSystemComponent>(
                      "FirstPersonExtras", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);

                ec->Class<FirstPersonControllerSystemComponent>(
                      "CameraCoupledChild", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);

#ifdef NETWORKFPC
                ec->Class<FirstPersonControllerSystemComponent>(
                      "NetworkFPC", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
#endif
            }
        }
    }

    void FirstPersonControllerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        provided.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
        provided.push_back(AZ_CRC_CE("CameraCoupledChildService"));
#ifdef NETWORKFPC
        provided.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void FirstPersonControllerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        incompatible.push_back(AZ_CRC_CE("FirstPersonExtrasService"));
        incompatible.push_back(AZ_CRC_CE("CameraCoupledChildService"));
#ifdef NETWORKFPC
        incompatible.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void FirstPersonControllerSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void FirstPersonControllerSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    FirstPersonControllerSystemComponent::FirstPersonControllerSystemComponent()
    {
        if (FirstPersonControllerInterface::Get() == nullptr)
        {
            FirstPersonControllerInterface::Register(this);
        }
    }

    FirstPersonControllerSystemComponent::~FirstPersonControllerSystemComponent()
    {
        if (FirstPersonControllerInterface::Get() == this)
        {
            FirstPersonControllerInterface::Unregister(this);
        }
    }

    void FirstPersonControllerSystemComponent::Init()
    {
    }

    void FirstPersonControllerSystemComponent::Activate()
    {
        FirstPersonControllerRequestBus::Handler::BusConnect();
        FirstPersonExtrasRequestBus::Handler::BusConnect();
        CameraCoupledChildRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
#ifdef NETWORKFPC
        NetworkFPCRequestBus::Handler::BusConnect();
        // Register multiplayer components
        RegisterMultiplayerComponents();
#endif
    }

    void FirstPersonControllerSystemComponent::Deactivate()
    {
#ifdef NETWORKFPC
        NetworkFPCRequestBus::Handler::BusDisconnect();
#endif
        AZ::TickBus::Handler::BusDisconnect();
        FirstPersonControllerRequestBus::Handler::BusDisconnect();
        FirstPersonExtrasRequestBus::Handler::BusDisconnect();
        CameraCoupledChildRequestBus::Handler::BusDisconnect();
    }

    void FirstPersonControllerSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace FirstPersonController
