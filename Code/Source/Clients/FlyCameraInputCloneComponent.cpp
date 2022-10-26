/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <Clients/FlyCameraInputCloneComponent.h>

#include <Cry_Math.h>
#include <ISystem.h>
#include <IConsole.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/Entity.h>

#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Gamepad/InputDeviceGamepad.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Input/Devices/Touch/InputDeviceTouch.h>
#include <AzFramework/Physics/CharacterBus.h>

#include <MathConversion.h>

#include <Atom/RPI.Public/ViewProviderBus.h>
#include <Atom/RPI.Public/View.h>
#include <AzFramework/Components/CameraBus.h>

using namespace AzFramework;
using namespace AZ::AtomBridge;

//////////////////////////////////////////////////////////////////////////////
namespace
{
    //////////////////////////////////////////////////////////////////////////
    int GenerateThumbstickTexture()
    {
        // [GFX TODO] Get Atom test fly cam virtual thumbsticks working on mobile
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    void ReleaseThumbstickTexture([[maybe_unused]] int textureId)
    {
        // [GFX TODO] Get Atom test fly cam virtual thumbsticks working on mobile
    }

    //////////////////////////////////////////////////////////////////////////
    void DrawThumbstick([[maybe_unused]] Vec2 initialPosition,
        [[maybe_unused]] Vec2 currentPosition,
        [[maybe_unused]] int textureId)
    {
        // [GFX TODO] Get Atom test fly cam virtual thumbsticks working on mobile
        // we do not have any 2D drawing capability like IDraw2d in Atom yet
    }
}

namespace FirstPersonController
{
    //////////////////////////////////////////////////////////////////////////////
    const AZ::Crc32 FlyCameraInputCloneComponent::UnknownInputChannelId("unknown_input_channel_id");
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("InputService", 0xd41af40c));
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::Reflect(AZ::ReflectContext* rc)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(rc);
        if (serializeContext)
        {
            serializeContext->Class<FlyCameraInputCloneComponent, AZ::Component>()
                ->Version(1)
                ->Field("Move Speed", &FlyCameraInputCloneComponent::m_moveSpeed)
                ->Field("Rotation Speed", &FlyCameraInputCloneComponent::m_rotationSpeed)
                ->Field("Mouse Sensitivity", &FlyCameraInputCloneComponent::m_mouseSensitivity)
                ->Field("Invert Rotation Input X", &FlyCameraInputCloneComponent::m_InvertRotationInputAxisX)
                ->Field("Invert Rotation Input Y", &FlyCameraInputCloneComponent::m_InvertRotationInputAxisY)
                ->Field("Is enabled", &FlyCameraInputCloneComponent::m_isEnabled);
    
            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                using namespace AZ::Edit::Attributes;
                editContext->Class<FlyCameraInputCloneComponent>("Fly Camera Input HACK", "The Fly Camera Input allows you to control the camera")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(Category, "First Person")
                    ->Attribute("Icon", "Icons/Components/FlyCameraInput.svg")
                    ->Attribute("ViewportIcon", "Icons/Components/Viewport/FlyCameraInput.svg")
                    ->Attribute("AutoExpand", true)
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(0, &FlyCameraInputCloneComponent::m_moveSpeed, "Move Speed", "Speed at which the camera moves")
                    ->Attribute("Min", 1.0f)
                    ->Attribute("Max", 100.0f)
                    ->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
                    ->DataElement(0, &FlyCameraInputCloneComponent::m_rotationSpeed, "Rotation Speed", "Speed at which the camera rotates")
                    ->Attribute("Min", 1.0f)
                    ->Attribute("Max", 100.0f)
                    ->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
                    ->DataElement(0, &FlyCameraInputCloneComponent::m_mouseSensitivity, "Mouse Sensitivity", "Mouse sensitivity factor")
                    ->Attribute("Min", 0.0f)
                    ->Attribute("Max", 1.0f)
                    ->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
                    ->DataElement(0, &FlyCameraInputCloneComponent::m_InvertRotationInputAxisX, "Invert Rotation Input X", "Invert rotation input x-axis")
                    ->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
                    ->DataElement(0, &FlyCameraInputCloneComponent::m_InvertRotationInputAxisY, "Invert Rotation Input Y", "Invert rotation input y-axis")
                    ->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &FlyCameraInputCloneComponent::m_isEnabled,
                        "Is Initially Enabled", "When checked, the fly cam input is enabled on activate, else it has to be specifically enabled.");
            }
        }
    
        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(rc);
        if (behaviorContext)
        {
            behaviorContext->EBus<FlyCameraInputBus>("FlyCameraInputBus")
                ->Event("SetIsEnabled", &FlyCameraInputBus::Events::SetIsEnabled)
                ->Event("GetIsEnabled", &FlyCameraInputBus::Events::GetIsEnabled);
        }
    }
    
    //////////////////////////////////////////////////////////////////////////////
    FlyCameraInputCloneComponent::~FlyCameraInputCloneComponent()
    {
        ReleaseThumbstickTexture(m_thumbstickTextureId);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::Init()
    {
        m_thumbstickTextureId = GenerateThumbstickTexture();
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::Activate()
    {
        InputChannelEventListener::Connect();
        AZ::TickBus::Handler::BusConnect();
        FlyCameraInputBus::Handler::BusConnect(GetEntityId());
    }
    
    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::Deactivate()
    {
        FlyCameraInputBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        InputChannelEventListener::Disconnect();
    }

    //////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        if (!m_isEnabled)
        {
            return;
        }
    
        AZ::Transform worldTransform = AZ::Transform::Identity();
        EBUS_EVENT_ID_RESULT(worldTransform, GetEntityId(), AZ::TransformBus, GetWorldTM);
    
        // Update movement
        const float moveSpeed = m_moveSpeed /** deltaTime*/;
        //const AZ::Vector3 right = worldTransform.GetBasisX();
        //const AZ::Vector3 forward = worldTransform.GetBasisY();
        //const AZ::Vector3 movement = (forward * m_movement.y) + (right * m_movement.x);
        //const AZ::Vector3 newPosition = worldTransform.GetTranslation() + (movement * moveSpeed);
        //worldTransform.SetTranslation(newPosition);

        const AZ::Vector3 move = AZ::Vector3(m_movement.x, m_movement.y, 0.f);
        const float currentHeading = GetEntity()->GetTransform()->
            GetWorldRotationQuaternion().GetEulerRadians().GetZ();

        const AZ::Vector3 velocity = AZ::Quaternion::CreateRotationZ(currentHeading).TransformVector(move) * moveSpeed;

        Physics::CharacterRequestBus::Event(GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick, velocity);
    
        const Vec2 invertedRotation(m_InvertRotationInputAxisX ? m_rotation.x : -m_rotation.x,
            m_InvertRotationInputAxisY ? m_rotation.y : -m_rotation.y);
    
        // Update rotation (not sure how to do this properly using just AZ::Quaternion)
        // Rotate yaw for the parent character entity
        const AZ::Quaternion worldOrientationCharacter = worldTransform.GetRotation();
        Ang3 rotationCharacter(AZQuaternionToLYQuaternion(worldOrientationCharacter));

        const Ang3 newRotationYaw = rotationCharacter + Ang3(0.f, 0.f, DEG2RAD(invertedRotation.x)) * m_rotationSpeed;
        const AZ::Quaternion newOrientation = LYQuaternionToAZQuaternion(Quat(newRotationYaw));
        worldTransform.SetRotation(newOrientation);

        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTM, worldTransform);

        // Rotate pitch for the child camera entity
        AZ::Entity* activeCameraEntity = GetActiveCamera();
        EBUS_EVENT_ID_RESULT(worldTransform, activeCameraEntity->GetId(), AZ::TransformBus, GetWorldTM);

        const AZ::Quaternion worldOrientationCam = worldTransform.GetRotation();
        Ang3 rotationCam(AZQuaternionToLYQuaternion(worldOrientationCam));

        if (rotationCam.x >= gf_PI/2)
            rotationCam.x = gf_PI/2;
        else if (rotationCam.x <= -gf_PI/2)
            rotationCam.x = -gf_PI/2;

        const Ang3 newRotationPitch = rotationCam + Ang3(DEG2RAD(invertedRotation.y), 0.f, 0.f) * m_rotationSpeed;
        const AZ::Quaternion newOrientationCam = LYQuaternionToAZQuaternion(Quat(newRotationPitch));
        worldTransform.SetRotation(newOrientationCam);

        EBUS_EVENT_ID(activeCameraEntity->GetId(), AZ::TransformBus, SetWorldTM, worldTransform);
    }

    AZ::Entity* FlyCameraInputCloneComponent::GetActiveCamera()
    {
        AZ::EntityId activeCameraId;
        Camera::CameraSystemRequestBus::BroadcastResult(activeCameraId,
            &Camera::CameraSystemRequestBus::Events::GetActiveCamera);

        auto ca = AZ::Interface<AZ::ComponentApplicationRequests>::Get();
        return ca->FindEntity(activeCameraId);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool FlyCameraInputCloneComponent::OnInputChannelEventFiltered(const InputChannel& inputChannel)
    {
        if (!m_isEnabled)
        {
            return false;
        }
    
        const InputDeviceId& deviceId = inputChannel.GetInputDevice().GetInputDeviceId();
        if (InputDeviceMouse::IsMouseDevice(deviceId))
        {
            OnMouseEvent(inputChannel);
        }
        else if (InputDeviceKeyboard::IsKeyboardDevice(deviceId))
        {
            OnKeyboardEvent(inputChannel);
        }
        else if (InputDeviceTouch::IsTouchDevice(deviceId))
        {
            const InputChannel::PositionData2D* positionData2D = inputChannel.GetCustomData<InputChannel::PositionData2D>();
            if (positionData2D)
            {
                float defaultViewWidth = GetViewWidth();
                float defaultViewHeight = GetViewHeight();
                const Vec2 screenPosition(positionData2D->m_normalizedPosition.GetX() * defaultViewWidth,
                                          positionData2D->m_normalizedPosition.GetY() * defaultViewHeight);
                OnTouchEvent(inputChannel, screenPosition);
            }
        }
        else if (AzFramework::InputDeviceGamepad::IsGamepadDevice(deviceId))
        {
            OnGamepadEvent(inputChannel);
        }
    
        return false;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::SetIsEnabled(bool isEnabled)
    {
        m_isEnabled = isEnabled;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    bool FlyCameraInputCloneComponent::GetIsEnabled()
    {
        return m_isEnabled;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    float Snap_s360(float val)
    {
        if (val < 0.0f)
        {
            val = f32(360.0f + fmodf(val, 360.0f));
        }
        else if (val >= 360.0f)
        {
            val = f32(fmodf(val, 360.0f));
        }
        return val;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnMouseEvent(const InputChannel& inputChannel)
    {
        const InputChannelId& channelId = inputChannel.GetInputChannelId();
        if (channelId == InputDeviceMouse::Movement::X)
        {
            m_rotation.x = Snap_s360(inputChannel.GetValue() * m_mouseSensitivity);
        }
        else if (channelId == InputDeviceMouse::Movement::Y)
        {
            m_rotation.y = Snap_s360(inputChannel.GetValue() * m_mouseSensitivity);
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnKeyboardEvent(const InputChannel& inputChannel)
    {
        if (gEnv && gEnv->pConsole && gEnv->pConsole->IsOpened())
        {
            return;
        }
    
        const InputChannelId& channelId = inputChannel.GetInputChannelId();
        if (channelId == InputDeviceKeyboard::Key::AlphanumericW)
        {
            m_movement.y = inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceKeyboard::Key::AlphanumericA)
        {
            m_movement.x = -inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceKeyboard::Key::AlphanumericS)
        {
            m_movement.y = -inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceKeyboard::Key::AlphanumericD)
        {
            m_movement.x = inputChannel.GetValue();
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnGamepadEvent(const InputChannel& inputChannel)
    {
        const InputChannelId& channelId = inputChannel.GetInputChannelId();
        if (channelId == InputDeviceGamepad::ThumbStickAxis1D::LX)
        {
            m_movement.x = inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceGamepad::ThumbStickAxis1D::LY)
        {
            m_movement.y = inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceGamepad::ThumbStickAxis1D::RX)
        {
            m_rotation.x = inputChannel.GetValue();
        }
        
        if (channelId == InputDeviceGamepad::ThumbStickAxis1D::RY)
        {
            m_rotation.y = inputChannel.GetValue();
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnTouchEvent(const InputChannel& inputChannel, const Vec2& screenPosition)
    {
        if (inputChannel.IsStateBegan())
        {
            const float screenCentreX = GetViewWidth() * 0.5f;
            if (screenPosition.x <= screenCentreX)
            {
                if (m_leftFingerId == UnknownInputChannelId)
                {
                    // Initiate left thumb-stick (movement)
                    m_leftDownPosition = screenPosition;
                    m_leftFingerId = inputChannel.GetInputChannelId().GetNameCrc32();
                    DrawThumbstick(m_leftDownPosition, screenPosition, m_thumbstickTextureId);
                }
            }
            else
            {
                if (m_rightFingerId == UnknownInputChannelId)
                {
                    // Initiate right thumb-stick (rotation)
                    m_rightDownPosition = screenPosition;
                    m_rightFingerId = inputChannel.GetInputChannelId().GetNameCrc32();
                    DrawThumbstick(m_rightDownPosition, screenPosition, m_thumbstickTextureId);
                }
            }
        }
        else if (inputChannel.GetInputChannelId().GetNameCrc32() == m_leftFingerId)
        {
            // Update left thumb-stick (movement)
            OnVirtualLeftThumbstickEvent(inputChannel, screenPosition);
        }
        else if (inputChannel.GetInputChannelId().GetNameCrc32() == m_rightFingerId)
        {
            // Update right thumb-stick (rotation)
            OnVirtualRightThumbstickEvent(inputChannel, screenPosition);
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnVirtualLeftThumbstickEvent(const InputChannel& inputChannel, const Vec2& screenPosition)
    {
        if (inputChannel.GetInputChannelId().GetNameCrc32() != m_leftFingerId)
        {
            return;
        }
    
        switch (inputChannel.GetState())
        {
            case InputChannel::State::Ended:
            {
                // Stop movement
                m_leftFingerId = UnknownInputChannelId;
                m_movement = ZERO;
            }
            break;
    
            case InputChannel::State::Updated:
            {
                // Calculate movement
                const float discRadius = GetViewWidth() * m_virtualThumbstickRadiusAsPercentageOfScreenWidth;
                const float distScalar = 1.0f / discRadius;
    
                Vec2 dist = screenPosition - m_leftDownPosition;
                dist *= distScalar;
    
                m_movement.x = AZ::GetClamp(dist.x, -1.0f, 1.0f);
                m_movement.y = AZ::GetClamp(-dist.y, -1.0f, 1.0f);
    
                DrawThumbstick(m_leftDownPosition, screenPosition, m_thumbstickTextureId);
            }
            break;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void FlyCameraInputCloneComponent::OnVirtualRightThumbstickEvent(const InputChannel& inputChannel, const Vec2& screenPosition)
    {
        if (inputChannel.GetInputChannelId().GetNameCrc32() != m_rightFingerId)
        {
            return;
        }
    
        switch (inputChannel.GetState())
        {
            case InputChannel::State::Ended:
            {
                // Stop rotation
                m_rightFingerId = UnknownInputChannelId;
                m_rotation = ZERO;
            }
            break;
    
            case InputChannel::State::Updated:
            {
                // Calculate rotation
                const float discRadius = GetViewWidth() * m_virtualThumbstickRadiusAsPercentageOfScreenWidth;
                const float distScalar = 1.0f / discRadius;
    
                Vec2 dist = screenPosition - m_rightDownPosition;
                dist *= distScalar;
    
                m_rotation.x = AZ::GetClamp(dist.x, -1.0f, 1.0f);
                m_rotation.y = AZ::GetClamp(dist.y, -1.0f, 1.0f);
    
                DrawThumbstick(m_rightDownPosition, screenPosition, m_thumbstickTextureId);
            }
            break;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    float FlyCameraInputCloneComponent::GetViewWidth() const
    {
        float viewWidth = 256.0f;
        Camera::CameraRequestBus::EventResult(viewWidth, GetEntityId(), &Camera::CameraRequestBus::Events::GetFrustumWidth);
        return viewWidth;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    float FlyCameraInputCloneComponent::GetViewHeight() const
    {
        float viewHeight = 256.0f;
        Camera::CameraRequestBus::EventResult(viewHeight, GetEntityId(), &Camera::CameraRequestBus::Events::GetFrustumHeight);
        return viewHeight;
    }
}
