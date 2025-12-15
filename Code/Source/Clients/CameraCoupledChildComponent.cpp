/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <Clients/CameraCoupledChildComponent.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace FirstPersonController
{
    using namespace StartingPointInput;

    void CameraCoupledChildComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<CameraCoupledChildComponent, AZ::Component>()
                // Enable Camera Coupled Child
                ->Field("Enable Camera Coupled Child", &CameraCoupledChildComponent::m_enable)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<CameraCoupledChildComponent>(
                      "Camera Coupled Child", "Couples the transform of a child of the First Person Controller entity to the camera")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->DataElement(
                        nullptr,
                        &CameraCoupledChildComponent::m_enable,
                        "Enable Camera Coupled Child",
                        "Sets whether the Camera Coupled Child component is enabled.");
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<CameraCoupledChildComponentNotificationBus>("CameraCoupledChildComponentNotificationBus")
                ->Handler<CameraCoupledChildComponentNotificationHandler>();

            bc->EBus<CameraCoupledChildComponentRequestBus>("CameraCoupledChildComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "controller")
                ->Attribute(AZ::Script::Attributes::Category, "Camera Coupled Child")
                ->Event("Get Enable Camera Coupled Child", &CameraCoupledChildComponentRequests::GetEnableCameraCoupledChild)
                ->Event("Get Enable Camera Coupled Child", &CameraCoupledChildComponentRequests::SetEnableCameraCoupledChild);

            bc->Class<CameraCoupledChildComponent>()->RequestBus("CameraCoupledChildComponentRequestBus");
        }
    }

    void CameraCoupledChildComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void CameraCoupledChildComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CameraCoupledChildComponent::OnEntityActivated(const AZ::EntityId& entityId)
    {
    }

    AZ::Entity* CameraCoupledChildComponent::GetActiveCamera() const
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId, &Camera::CameraSystemRequestBus::Events::GetActiveCamera);
        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }

    void CameraCoupledChildComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
    }

    void CameraCoupledChildComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CameraCoupledChildService"));
    }

    void CameraCoupledChildComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CameraCoupledChildService"));
    }

    void CameraCoupledChildComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput((deltaTime + m_prevDeltaTime) / 2.f);
        m_prevDeltaTime = deltaTime;
    }

    void CameraCoupledChildComponent::ProcessInput(const float& deltaTime)
    {
        if (!m_enable)
            return;
    }

    // Request Bus getter and setter methods for use in scripts
    bool CameraCoupledChildComponent::GetEnableCameraCoupledChild() const
    {
        return m_enable;
    }
    void CameraCoupledChildComponent::SetEnableCameraCoupledChild(const bool& new_enable)
    {
        m_enable = new_enable;
    }
} // namespace FirstPersonController
