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
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<CameraCoupledChildComponent>(
                      "Camera Coupled Child", "Couples the transform of a child of the First Person Controller entity to the camera")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "First Person Controller")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ");
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
                ->Event("Set Enable Camera Coupled Child", &CameraCoupledChildComponentRequests::SetEnableCameraCoupledChild)
                ->Event("Get Initial Z Offset", &CameraCoupledChildComponentRequests::GetInitialZOffset)
                ->Event("Set Initial Z Offset", &CameraCoupledChildComponentRequests::SetInitialZOffset);

            bc->Class<CameraCoupledChildComponent>()->RequestBus("CameraCoupledChildComponentRequestBus");
        }
    }

    void CameraCoupledChildComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void CameraCoupledChildComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void CameraCoupledChildComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the FirstPersonControllerComponent object and its members
        AZ::EntityId characterEntityId;
        AZ::TransformBus::EventResult(characterEntityId, GetEntityId(), &AZ::TransformBus::Events::GetParentId);
        AZ::Entity* characterEntity;
        AZ::ComponentApplicationBus::BroadcastResult(characterEntity, &AZ::ComponentApplicationBus::Events::FindEntity, characterEntityId);
        if (characterEntity)
        {
            m_firstPersonControllerObject = characterEntity->FindComponent<FirstPersonControllerComponent>();
            m_firstPersonExtrasObject = characterEntity->FindComponent<FirstPersonExtrasComponent>();

            if (m_firstPersonControllerObject != nullptr &&
                (m_firstPersonControllerObject->m_isAutonomousClient || m_firstPersonControllerObject->m_isHost))
                m_initialZOffset = GetEntity()->GetTransform()->GetWorldTranslation().GetZ();
            else
                m_initialZOffset = GetEntity()->GetTransform()->GetLocalTranslation().GetZ();
        }
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

    void CameraCoupledChildComponent::CoupleChildToCamera()
    {
        // Get the camera translation, based on whether headbob is enabled
        AZ::Vector3 cameraTranslation = m_firstPersonExtrasObject->m_cameraTranslationWithoutHeadbob;
        if (m_firstPersonExtrasObject == nullptr || !m_firstPersonExtrasObject->m_headbobEnabled)
            cameraTranslation = GetActiveCamera()->GetTransform()->GetWorldTranslation();

        // Get the Z offset
        const float childZOffset = m_firstPersonControllerObject->m_eyeHeight - m_initialZOffset;

        // Set the child's yaw rotation to be the same as the camera
        AZ::Vector3 zPositiveDirection = AZ::Vector3::CreateAxisZ();
        const bool isCameraChildOfCharacter = m_firstPersonControllerObject->IsCameraChildOfCharacter();
        if (isCameraChildOfCharacter)
        {
            const AZ::Vector3 childRotation = GetEntity()->GetTransform()->GetLocalRotation();
            GetEntity()->GetTransform()->SetLocalRotation(
                AZ::Vector3(childRotation.GetX(), childRotation.GetY(), m_firstPersonControllerObject->m_cameraYaw));
        }
        else
        {
            const AZ::Vector3 childRotation = GetEntity()->GetTransform()->GetWorldRotation();
            GetEntity()->GetTransform()->SetWorldRotation(
                AZ::Vector3(childRotation.GetX(), childRotation.GetY(), m_firstPersonControllerObject->m_cameraYaw));
            zPositiveDirection = m_firstPersonControllerObject->m_sphereCastsAxisDirectionPose;
        }

        // Calculate the child entity's new translation, based on whether the character is crouching
        AZ::Vector3 newChildTranslation = AZ::Vector3::CreateZero();
        if (m_firstPersonControllerObject->m_crouched)
            newChildTranslation = cameraTranslation + (m_firstPersonControllerObject->m_crouchDistance - childZOffset) * zPositiveDirection;
        else if (m_firstPersonControllerObject->m_crouchingDownMove || m_firstPersonControllerObject->m_standingUpMove)
            newChildTranslation =
                cameraTranslation + (-m_firstPersonControllerObject->m_cameraLocalZTravelDistance - childZOffset) * zPositiveDirection;
        else
            newChildTranslation = cameraTranslation - childZOffset * zPositiveDirection;

        // Set the new translation
        if (isCameraChildOfCharacter)
            GetEntity()->GetTransform()->SetLocalTranslation(newChildTranslation);
        else
            GetEntity()->GetTransform()->SetWorldTranslation(newChildTranslation);
    }

    void CameraCoupledChildComponent::ProcessInput([[maybe_unused]] const float& deltaTime)
    {
        bool networkFPCEnabled = true;
        FirstPersonControllerComponentRequestBus::BroadcastResult(
            networkFPCEnabled, &FirstPersonControllerComponentRequestBus::Events::GetLocallyEnableNetworkFPC);
        if (!m_enable || m_firstPersonControllerObject == nullptr || !m_firstPersonControllerObject->m_cameraSmoothFollow ||
            (networkFPCEnabled && !m_firstPersonControllerObject->m_isAutonomousClient && !m_firstPersonControllerObject->m_isHost))
            return;

        CoupleChildToCamera();
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
    float CameraCoupledChildComponent::GetInitialZOffset() const
    {
        return m_initialZOffset;
    }
    void CameraCoupledChildComponent::SetInitialZOffset(const float& new_initialZOffset)
    {
        m_initialZOffset = new_initialZOffset;
    }
} // namespace FirstPersonController
