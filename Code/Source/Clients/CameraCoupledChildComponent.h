/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once
#include <FirstPersonController/CameraCoupledChildComponentBus.h>
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#include <FirstPersonController/FirstPersonExtrasComponentBus.h>

#include <Clients/FirstPersonControllerComponent.h>
#include <Clients/FirstPersonExtrasComponent.h>

#include <AzFramework/Components/CameraBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>

#include <AzFramework/Components/CameraBus.h>

namespace FirstPersonController
{
    class NetworkFPC;

    class CameraCoupledChildComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AZ::EntityBus::Handler
        , public CameraCoupledChildComponentRequestBus::Handler
    {
        friend class FirstPersonControllerComponent;
        friend class FirstPersonExtrasComponent;
        friend class NetworkFPCController;

    public:
        AZ_COMPONENT(CameraCoupledChildComponent, "{76b7172d-6196-4e4b-b22c-4afbb20580be}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::EntityBus interface
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // CameraCoupledChildRequestBus
        bool GetEnableCameraCoupledChild() const override;
        void SetEnableCameraCoupledChild(const bool& new_enable) override;
        float GetInitialZOffset() const override;
        void SetInitialZOffset(const float& new_initialZOffset) override;

    private:
        // Input event assignment and notification bus connection
        void AssignConnectInputEvents();

        AZ::Entity* GetActiveCamera() const;
        AZ::Entity* GetEntityPtr(AZ::EntityId pointer) const;

        // Called on each tick
        void ProcessInput(const float& deltaTime);

        // Function that does the coupling to the camera
        void CoupleChildToCamera(const float& deltaTime);

        // FirstPersonControllerComponent and FirstPersonExtrasComponent objects
        FirstPersonControllerComponent* m_firstPersonControllerObject = nullptr;
        FirstPersonExtrasComponent* m_firstPersonExtrasObject = nullptr;

        // Enable/disable this component
        bool m_enable = true;

        // Previous deltaTime
        float m_prevDeltaTime = 1.f / 60.f;

        // The inital Z offset of the child entity with respect to the parent character entity
        float m_initialZOffset = 0.f;

        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);
    };
} // namespace FirstPersonController
