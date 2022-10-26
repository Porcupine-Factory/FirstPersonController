/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <Cry_Math.h>
#include <AtomBridge/FlyCameraInputBus.h>

namespace FirstPersonController
{
    /// This is based on the FlyCameraInputCloneComponent in SamplesProject and is just used to test the CameraComponent
    class FlyCameraInputCloneComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::InputChannelEventListener
        , public AZ::AtomBridge::FlyCameraInputBus::Handler
    {
    public:
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void Reflect(AZ::ReflectContext* rc);

        AZ_COMPONENT(FlyCameraInputCloneComponent, "{63e1ae7b-6a7c-41ca-aeca-08b5d67a66fd}");
        virtual ~FlyCameraInputCloneComponent();

        // AZ::Component
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // AZ::TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        // AzFramework::InputChannelEventListener
        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        // FlyCameraInputCloneInterface
        void SetIsEnabled(bool isEnabled) override;
        bool GetIsEnabled() override;

    private:
        void OnMouseEvent(const AzFramework::InputChannel& inputChannel);
        void OnKeyboardEvent(const AzFramework::InputChannel& inputChannel);
        void OnGamepadEvent(const AzFramework::InputChannel& inputChannel);
        void OnTouchEvent(const AzFramework::InputChannel& inputChannel, const Vec2& screenPosition);
        void OnVirtualLeftThumbstickEvent(const AzFramework::InputChannel& inputChannel, const Vec2& screenPosition);
        void OnVirtualRightThumbstickEvent(const AzFramework::InputChannel& inputChannel, const Vec2& screenPosition);

        float GetViewWidth() const;
        float GetViewHeight() const;
        AZ::Entity* GetActiveCamera();

        static const AZ::Crc32 UnknownInputChannelId;

        // Editable Properties
        float m_moveSpeed = 4.0f;
        float m_rotationSpeed = 5.0f;

        float m_mouseSensitivity = 0.025f;
        float m_virtualThumbstickRadiusAsPercentageOfScreenWidth = 0.1f;

        bool m_InvertRotationInputAxisX = false;
        bool m_InvertRotationInputAxisY = false;

        bool m_isEnabled = true;

        // Run-time Properties
        Vec2 m_movement = ZERO;
        Vec2 m_rotation = ZERO;

        Vec2 m_leftDownPosition = ZERO;
        AZ::Crc32 m_leftFingerId = UnknownInputChannelId;

        Vec2 m_rightDownPosition = ZERO;
        AZ::Crc32 m_rightFingerId = UnknownInputChannelId;

        int m_thumbstickTextureId = 0;
    };
}
